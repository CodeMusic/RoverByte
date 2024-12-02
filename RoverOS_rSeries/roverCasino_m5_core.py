import M5
from M5 import *
import time
import random

# Colors
WHITE = 0xFFFFFF
BLACK = 0x000000
RED = 0xFF0000
GREEN = 0x00FF00
BLUE = 0x0000FF
GOLD = 0xFFD700
PINK = 0xFF69B4
PURPLE = 0x800080
CLOVER_GREEN = 0x00DD00
SILVER = 0xC0C0C0
LIGHT_SILVER = 0xDCDCDC  # Lighter shade for title area
DARK_GREEN = const(0x0320)  # Add this with your other color definitions

# Game state
credits = 250
bet = 10
symbols = ['R', '7', '8', '*']  # Question mark only for initial state
initial_symbols = ['?', 'R', '7', '8', '*']  # Full list including ? for initial display
reels = [0, 0, 0]  # Will show '?' at index 0
color_level = 0  # 0=B&W, 1=Red, 2=Blue, 3=Green, 4=Gold, 5=Full Color
luck_level = 0  # 0-28% (4 leaves x 7%)
clover_leaves = 0  # 0-4 leaves
last_lucky_win = 0  # Time tracking for luck duration
last_win_time = time.ticks_ms()
losing_streak = 0
initial_state = True  # Add this flag to track initial state
message_showing = False

def get_symbol_color(symbol):
    if color_level == 0:
        return WHITE  # Start monochrome
    
    # Add colors as you win
    if symbol == 'R':
        return RED
    elif symbol == '7':
        return GOLD
    elif symbol == '8':
        return BLUE
    elif symbol == '*':
        return GREEN
    elif symbol == '?':
        return SILVER
    return WHITE

def setup():
    global initial_state
    M5.begin()
    M5.Lcd.setBrightness(100)
    draw_machine()
    
    # Initialize with question marks
    for i in range(3):
        x = 40 + (i * 85)
        M5.Lcd.fillRect(x, 90, 70, 60, WHITE)
        draw_symbol('?', x, 90)
    
    update_display()
    initial_state = True  # Set initial state flag

def draw_machine():
    # Main background
    M5.Lcd.clear(BLACK)
    
    # Calculate text width for centering
    text_width = 100  # Approximate width of "ROVER SLOTS" text
    screen_width = 320
    rect_width = text_width + 20  # Add padding
    rect_x = (screen_width - rect_width) // 2  # Center position
    
    # Title background - centered and just around text
    M5.Lcd.fillRect(rect_x, 5, rect_width, 50, SILVER)
    
    # Main game panel with dividers
    M5.Lcd.fillRect(20, 75, 280, 155, BLACK)
    M5.Lcd.fillRect(22, 77, 276, 151, SILVER)
    M5.Lcd.fillRect(30, 85, 260, 85, BLACK)
    
    # Draw reel dividers
    M5.Lcd.fillRect(125, 85, 2, 85, SILVER)  # Left divider
    M5.Lcd.fillRect(210, 85, 2, 85, SILVER)  # Right divider
    
    # Title text - centered in background
    M5.Lcd.setTextSize(2)
    title_color = BLACK if color_level == 0 else [BLACK, RED, BLUE, GREEN, GOLD, PURPLE][color_level]
    M5.Lcd.setTextColor(title_color, SILVER)
    text_x = rect_x + 10  # Add small padding from rect edge
    M5.Lcd.drawString("ROVER", text_x, 15)
    M5.Lcd.drawString("SLOTS", text_x, 35)
    
    # Draw Rover first
    draw_mini_rover(x=30, y=5)
    
    # Draw clover last
    draw_clover(x=300, y=25)

def draw_symbol(symbol, x, y):
    M5.Lcd.setTextSize(4)
    
    if symbol == '?' and initial_state:
        # Question mark in initial state
        M5.Lcd.fillRect(x, y, 70, 60, WHITE)
        M5.Lcd.setTextColor(SILVER)
        for dx in [-1, 0, 1]:
            for dy in [-1, 0, 1]:
                if dx != 0 or dy != 0:
                    M5.Lcd.drawString(symbol, x+20+dx, y+15+dy)
        M5.Lcd.setTextColor(BLACK)
        M5.Lcd.drawString(symbol, x+20, y+15)
    else:
        # Regular game symbols
        M5.Lcd.fillRect(x, y, 70, 60, BLACK)
        symbol_color = get_symbol_color(symbol)
        
        # Black outline
        M5.Lcd.setTextColor(SILVER)
        for dx in [-1, 0, 1]:
            for dy in [-1, 0, 1]:
                if dx != 0 or dy != 0:
                    M5.Lcd.drawString(symbol, x+20+dx, y+15+dy)
        
        # Main symbol
        M5.Lcd.setTextColor(symbol_color)
        M5.Lcd.drawString(symbol, x+20, y+15)

def update_display():
    # Update reels
    for i in range(3):
        x = 40 + (i * 85)
        M5.Lcd.fillRect(x, 90, 70, 60, WHITE)
        draw_symbol(symbols[reels[i]], x, 90)
    
    # Update score - adjusted position
    M5.Lcd.fillRect(30, 185, 260, 40, WHITE if color_level == 0 else GOLD)  # Moved up by 5px
    M5.Lcd.setTextSize(3)
    M5.Lcd.setTextColor(BLACK)
    M5.Lcd.drawString(f"${credits} B:{bet}", 40, 190)  # Moved up by 5px

def play_sound(sound_type):
    M5.Speaker.setVolume(8)
    
    if sound_type == 'spin':
        M5.Speaker.tone(440, 100)  # Simple A4 note
    elif sound_type == 'stop':
        M5.Speaker.tone(880, 50)   # Simple A5 note
    elif sound_type == 'lose':
        M5.Speaker.tone(220, 200)  # Simple A3 note
    elif sound_type == 'win':
        M5.Speaker.tone(880, 200)  # Simple high note
    elif sound_type == 'rover_jackpot':
        M5.Speaker.tone(1320, 300)  # Simple high note
    elif sound_type == 'super_jackpot':
        M5.Speaker.tone(1760, 400)  # Simple highest note
    
    time.sleep_ms(50)  # Brief pause after sound
    M5.Speaker.stop()  # Using stop() instead of mute()

def clear_messages():
    # Only clear the message areas if they're being used
    global message_showing
    if message_showing:
        # Clear LUCK+ message area
        M5.Lcd.fillRect(240, 45, 80, 25, BLACK)
        # Clear win message area
        M5.Lcd.fillRect(30, 170, 260, 30, BLACK)
        message_showing = False

def show_message(text, x, y, color=WHITE, bg_color=BLACK):
    global message_showing
    M5.Lcd.fillRect(x, y, 260, 30, bg_color)
    M5.Lcd.setTextSize(2)
    M5.Lcd.setTextColor(color, bg_color)
    M5.Lcd.drawString(text, x + 10, y + 5)
    message_showing = True

def spin():
    global credits, initial_state, clover_leaves
    
    clear_messages()
    credits -= bet
    update_display()
    
    # Empty one clover leaf if any are filled
    if clover_leaves > 0:
        clover_leaves -= 1
        draw_clover()
    
    play_sound('spin')
    
    # Clear all reels to BLACK first
    for i in range(3):
        x = 40 + (i * 85)
        M5.Lcd.fillRect(x, 90, 70, 60, BLACK)
    
    # Show excited Rover
    draw_mini_rover(expression='excited')
    
    # No longer in initial state after first spin
    initial_state = False
    
    # Staggered reel stops
    for reel in range(3):
        for _ in range(6):
            reels[reel] = random.randint(0, len(symbols)-1)  # Use game symbols (no '?')
            x = 40 + (reel * 85)
            M5.Lcd.fillRect(x, 90, 70, 60, WHITE)
            draw_symbol(symbols[reels[reel]], x, 90)
            time.sleep(0.1)
        
        play_sound('stop')
        time.sleep(0.2)
    
    check_win()
    update_display()

def show_win_message(win_type):
    M5.Lcd.fillRect(30, 90, 260, 90, BLACK)
    
    # Larger win messages
    M5.Lcd.setTextSize(4)  # Increased size
    M5.Lcd.setTextColor(GOLD, BLACK)
    
    if win_type == 'rover':
        M5.Lcd.drawString("ROVER", 70, 100)
        M5.Lcd.drawString("JACKPOT!", 50, 140)
    elif win_type == 'super':
        M5.Lcd.drawString("SUPER", 80, 100)
        M5.Lcd.drawString("JACKPOT!", 50, 140)
    else:
        M5.Lcd.drawString("WINNER!", 60, 115)
    
    time.sleep(1)
    
    # Redraw reels
    for i in range(3):
        x = 40 + (i * 85)
        M5.Lcd.fillRect(x, 90, 70, 60, WHITE)
        draw_symbol(symbols[reels[i]], x, 90)

def check_win():
    global credits, color_level, luck_level, clover_leaves, last_win_time, losing_streak, message_showing
    
    if initial_state:
        return
        
    lucky_boost = random.randint(0, 100) < luck_level
    
    # Fix win checking - compare actual symbols
    symbol1 = symbols[reels[0]]
    symbol2 = symbols[reels[1]]
    symbol3 = symbols[reels[2]]
    is_win = (symbol1 == symbol2 == symbol3)
    
    if is_win or lucky_boost:
        message_showing = True
        if is_win:
            winning_symbol = symbol1
            if winning_symbol == 'R':
                win = bet * 10
                play_sound('rover_jackpot')
                show_win_message('rover')
            elif winning_symbol == '7':
                win = bet * 15
                play_sound('super_jackpot')
                show_win_message('super')
            else:  # '8' or '*'
                win = bet * 3
                play_sound('win')
                show_win_message('normal')
            
            credits += win
            last_win_time = time.ticks_ms()
            losing_streak = 0
            
            # Fill clover on win
            clover_leaves = 4
            draw_clover()
            
            if win >= 30 and color_level < 5:
                color_level += 1
                draw_machine()
            
            flash_win(win)
            draw_mini_rover(expression='lucky' if lucky_boost else 'win')
    else:
        play_sound('lose')
        losing_streak += 1
        draw_mini_rover(expression='sad')

def flash_win(amount):
    global message_showing
    message_showing = True
    flash_colors = [WHITE, SILVER] if color_level == 0 else [GREEN, GOLD]
    for _ in range(3):
        for color in flash_colors:
            show_message(f"WIN: ${amount}!", 30, 170, BLACK, color)
            time.sleep(0.2)

def draw_clover(x=300, y=25):
    # Start monochrome, add color as you win
    clover_color = SILVER if color_level == 0 else GREEN
    stem_color = SILVER if color_level == 0 else BLACK
    
    # Draw stem
    M5.Lcd.fillRect(x-1, y+8, 3, 15, stem_color)
    
    # Draw leaves based on count
    leaf_positions = [(0,0), (-8,-8), (8,-8), (0,-16)]
    for i in range(4):
        dx, dy = leaf_positions[i]
        if i < clover_leaves:
            # Active leaf (filled)
            M5.Lcd.fillCircle(x+dx, y+dy, 5, clover_color)
            M5.Lcd.drawCircle(x+dx, y+dy, 5, BLACK)
        else:
            # Inactive leaf (hollow with black center)
            M5.Lcd.drawCircle(x+dx, y+dy, 5, SILVER)
            M5.Lcd.fillCircle(x+dx, y+dy, 3, BLACK)  # Black center

def update_luck():
    global luck_level, clover_leaves, losing_streak
    
    current_time = time.ticks_ms()
    
    # Check for long losing streak (30 seconds without win)
    if time.ticks_diff(current_time, last_win_time) > 30000:
        if luck_level < 49:  # If not at max luck
            luck_level = min(49, luck_level + 7)  # Add 7%
            # Show time-based luck bonus
            M5.Lcd.setTextSize(2)
            M5.Lcd.setTextColor(GREEN, BLACK)
            M5.Lcd.drawString(f"LUCK +7%", 240, 50)  # Moved up another 10px
            time.sleep(0.5)
    
    # Only update leaves if we have active ones
    if clover_leaves > 0:
        # Check if luck has expired (10 seconds since last update)
        if time.ticks_diff(current_time, last_win_time) > 10000:
            clover_leaves -= 1
            luck_level = max(7, clover_leaves * 17)  # Increased from 7 to 17
            draw_clover()
            
            # Update mini Rover expression
            if clover_leaves == 0:
                draw_mini_rover(expression='normal')
            elif luck_level > 28:
                draw_mini_rover(expression='lucky')

def add_luck():
    global luck_level, clover_leaves, last_lucky_win
    
    # Reset timer
    last_lucky_win = time.ticks_ms()
    
    # Add luck and leaves
    if clover_leaves < 4:
        clover_leaves += 1
    luck_level = clover_leaves * 7
    
    # Visual feedback
    draw_clover()
    M5.Lcd.setTextSize(1)
    M5.Lcd.setTextColor(CLOVER_GREEN)
    M5.Lcd.drawString(f"LUCK +{luck_level}%", 240, 70)
    time.sleep(0.5)

def draw_mini_rover(x=30, y=5, expression='normal'):
    # Clear previous Rover area first
    M5.Lcd.fillRect(x-5, y-5, 60, 60, BLACK)
    
    # Black outline for entire rover
    M5.Lcd.fillRoundRect(x-2, y-2, 52, 52, 12, BLACK)
    # Main head
    M5.Lcd.fillRoundRect(x, y, 50, 50, 12, WHITE)
    
    # Ears moved more to the right
    if expression == 'win' or expression == 'lucky':
        # Perked up ears
        # Black outline
        M5.Lcd.fillTriangle(x+2, y-8, x+8, y+2, x+14, y+2, BLACK)  # Left ear moved right
        M5.Lcd.fillTriangle(x+26, y+2, x+32, y+2, x+38, y-8, BLACK)  # Right ear moved right
        # White fill
        M5.Lcd.fillTriangle(x+3, y-7, x+8, y+2, x+13, y+2, WHITE)
        M5.Lcd.fillTriangle(x+27, y+2, x+32, y+2, x+37, y-7, WHITE)
    else:
        # Regular ears
        # Black outline
        M5.Lcd.fillTriangle(x+2, y-2, x+8, y+7, x+14, y+7, BLACK)  # Left ear moved right
        M5.Lcd.fillTriangle(x+26, y+7, x+32, y+7, x+38, y-2, BLACK)  # Right ear moved right
        # White fill
        M5.Lcd.fillTriangle(x+3, y-1, x+8, y+6, x+13, y+6, WHITE)
        M5.Lcd.fillTriangle(x+27, y+6, x+32, y+6, x+37, y-1, WHITE)
    
    # Rest of face
    M5.Lcd.fillRoundRect(x+6, y+13, 37, 13, 3, SILVER)
    
    # Eyes based on expression
    if expression == 'lucky':
        # Star-shaped eyes
        for i in range(-3, 4):
            M5.Lcd.drawPixel(x+17+i, y+20, BLACK)
            M5.Lcd.drawPixel(x+17, y+20+i, BLACK)
            M5.Lcd.drawPixel(x+33+i, y+20, BLACK)
            M5.Lcd.drawPixel(x+33, y+20+i, BLACK)
    elif expression == 'win':
        # Happy eyes
        M5.Lcd.drawLine(x+13, y+20, x+20, y+17, BLACK)
        M5.Lcd.drawLine(x+20, y+17, x+27, y+20, BLACK)
        M5.Lcd.drawLine(x+30, y+20, x+37, y+17, BLACK)
        M5.Lcd.drawLine(x+37, y+17, x+44, y+20, BLACK)
    else:
        # Regular eyes
        M5.Lcd.fillCircle(x+17, y+20, 4, SILVER)
        M5.Lcd.fillCircle(x+17, y+20, 3, BLACK)
        M5.Lcd.fillCircle(x+33, y+20, 4, SILVER)
        M5.Lcd.fillCircle(x+33, y+20, 3, BLACK)
    
    # Nose
    M5.Lcd.fillRect(x+22, y+30, 6, 6, BLACK)

def handle_cash_out():
    global credits, color_level, clover_leaves, initial_state
    if credits > 0:
        # Show cash out message
        message_showing = True
        M5.Lcd.fillRect(30, 170, 260, 30, GOLD)
        M5.Lcd.setTextSize(2)
        M5.Lcd.setTextColor(BLACK)
        M5.Lcd.drawString(f"CASHED OUT: ${credits}!", 40, 175)
        time.sleep(2)
        
        # Reset game state
        credits = 250  # Reset to starting credits
        color_level = 0  # Reset to monochrome
        clover_leaves = 4  # Reset clover
        initial_state = True  # Back to initial state
        
        # Redraw everything in initial state
        clear_messages()
        draw_machine()
        
        # Reset reels to question marks
        for i in range(3):
            x = 40 + (i * 85)
            M5.Lcd.fillRect(x, 90, 70, 60, WHITE)
            draw_symbol('?', x, 90)
        
        update_display()

# Main setup
setup()

# Main loop
while True:
    M5.update()
    update_luck()  # Check if luck has expired
    
    if M5.BtnA.wasPressed():  # Bet
        bet = (bet % 50) + 10
        update_display()
        
    elif M5.BtnB.wasPressed() and credits >= bet:  # Spin
        spin()
        
    elif M5.BtnC.wasPressed():  # Cash out
        handle_cash_out()
    
    time.sleep(0.1)