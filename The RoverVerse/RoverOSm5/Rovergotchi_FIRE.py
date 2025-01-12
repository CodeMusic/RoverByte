import M5
from M5 import *
import time
import math
import random

M5.begin()
M5.Speaker.setVolume(1)  # Set speaker volume

# Initialize the screen with basic commands
M5.Lcd.clear()
M5.Lcd.fillScreen(0x87CEEB)
M5.Lcd.print("Starting...")  # Test if basic printing works

# Define global variables for game state
hunger = 100
happiness = 100
health = 100
poopometer = 0
game_running = False
jumping = False
jump_pos = 0

# Pet movement variables
pet_x = M5.Lcd.width() // 2
pet_y = M5.Lcd.height() // 2
move_direction = 2  # Increased speed
move_timer = 0
move_range = 50  # How far the pet can move from center

# Sun and ground
sun_x = M5.Lcd.width() * 0.75  # Start at 3/4 of screen width
sun_radius = 15
sun_speed = 0.3  # Even slower movement
sun_position = 0
sun_or_moon = False

# Menu state
MAIN_MENU = ["Food", "Games", "Clean", "Sleep", "Stats"]
SUB_MENUS = {
    "Food": ["Apple", "Water"],
    "Games": ["Jump Game", "Run Game"],
}
current_menu = MAIN_MENU
selected_item = 0
menu_opened = False
in_sub_menu = False

# Functions
def draw_horizon():
    """Draw the horizon line and grass."""
    # Draw green grass for bottom half
    M5.Lcd.fillRect(0, 
                   M5.Lcd.height() // 2, 
                   M5.Lcd.width(), 
                   M5.Lcd.height() // 2, 
                   0x228B22)  # Forest Green
    
    # Draw horizon line
    M5.Lcd.drawLine(0, 
                   M5.Lcd.height() // 2, 
                   M5.Lcd.width(), 
                   M5.Lcd.height() // 2, 
                   0x000000)  # Black line for horizon

def draw_sun_or_moon():
    """Animate the sun or moon."""
    global sun_x, sun_or_moon, sun_position
    
    sun_position += sun_speed
    sun_x = (M5.Lcd.width() // 2) + int(sun_position)
    
    if sun_x > M5.Lcd.width() + sun_radius:
        sun_x = M5.Lcd.width() // 2
        sun_position = 0
        sun_or_moon = not sun_or_moon

    if sun_or_moon:  # Draw moon
        M5.Lcd.fillCircle(sun_x, 30, sun_radius, 0xFFFFFF)
        M5.Lcd.fillCircle(sun_x - 5, 30, sun_radius, 0x87CEEB)
    else:  # Draw sun
        M5.Lcd.fillCircle(sun_x, 30, sun_radius, 0xFFD700)

def display_stats():
    """Display pet stats in a black box."""
    # Draw black background box
    M5.Lcd.fillRect(5, 5, 150, 70, 0x000000)
    
    # Draw stats with white text
    M5.Lcd.setTextColor(0xFFFFFF)
    M5.Lcd.setTextSize(2)
    M5.Lcd.setCursor(10, 10)
    M5.Lcd.print(f"Hunger: {int(hunger)}")
    M5.Lcd.setCursor(10, 30)
    M5.Lcd.print(f"Happy: {int(happiness)}")
    M5.Lcd.setCursor(10, 50)
    M5.Lcd.print(f"Health: {int(health)}")

def update_pet_stats():
    """Update pet stats over time."""
    global hunger, happiness, health, poopometer

    hunger -= 0.05
    happiness -= 0.03
    health -= 0.02 + poopometer * 0.01
    poopometer += 0.01

    if hunger < 20 or happiness < 20 or health < 20:
        M5.Lcd.setTextColor(0xFF0000)
        M5.Lcd.setCursor(10, 70)
        M5.Lcd.print("Notification!")
        M5.Lcd.setTextColor(0xFFFFFF)

def reset_game():
    """Reset the game state."""
    global hunger, happiness, health, poopometer
    hunger = 100
    happiness = 100
    health = 100
    poopometer = 0

def game_logic():
    """Handle game mechanics."""
    global jumping, jump_pos

    if jumping:
        jump_pos += 5
        if jump_pos > 50:
            jumping = False
            jump_pos = 0

def button_actions():
    """Handle button actions."""
    global game_running, jumping

    if M5.BtnA.wasPressed():
        jumping = True  # Simulate jump
    if M5.BtnB.wasPressed():
        reset_game()  # Reset game
    if M5.BtnC.wasPressed():
        game_running = not game_running  # Pause/Resume

def handle_menu_navigation():
    """Handle navigation of the menu using the buttons."""
    global current_menu, selected_item, menu_opened, in_sub_menu

    if M5.BtnA.wasPressed():  # Left button (Select)
        play_button_sound("select")
        if not menu_opened:
            menu_opened = True
            M5.Lcd.clear()
        elif menu_opened:
            selected_item = (selected_item + 1) % len(current_menu)

    if M5.BtnB.wasPressed() and menu_opened:  # Middle button (Menu)
        play_button_sound("menu")
        if not in_sub_menu and current_menu[selected_item] in SUB_MENUS:
            current_menu = SUB_MENUS[current_menu[selected_item]]
            selected_item = 0
            in_sub_menu = True
        else:
            handle_menu_action(current_menu[selected_item])
            if in_sub_menu:
                current_menu = MAIN_MENU
                selected_item = 0
                in_sub_menu = False
                menu_opened = False

    if M5.BtnC.wasPressed() and menu_opened:  # Right button (Cancel)
        play_button_sound("cancel")
        if in_sub_menu:
            current_menu = MAIN_MENU
            selected_item = 0
            in_sub_menu = False
        else:
            menu_opened = False

def handle_menu_action(action):
    """Handle the selected menu action."""
    global hunger, happiness, health, poopometer
    
    M5.Lcd.clear()
    M5.Lcd.setCursor(10, 10)
    
    if action == "Apple":
        hunger += 20
        M5.Lcd.print("Eating apple...")
    elif action == "Water":
        hunger += 10
        health += 5
        M5.Lcd.print("Drinking water...")
    elif action == "Clean":
        poopometer = 0
        health += 10
        M5.Lcd.print("Cleaning...")
    elif action == "Sleep":
        health += 20
        happiness += 10
        M5.Lcd.print("Sleeping...")
    elif action == "Jump Game":
        happiness += 15
        M5.Lcd.print("Playing jump game...")
    elif action == "Run Game":
        happiness += 20
        hunger -= 10
        M5.Lcd.print("Playing run game...")
    
    # Cap stats at 100
    hunger = min(100, hunger)
    happiness = min(100, happiness)
    health = min(100, health)
    
    time.sleep(1)  # Show action message briefly

def draw_character(x, y):
    """Draw a more pet-like character."""
    # Body
    M5.Lcd.fillCircle(x, y, 15, 0xFFFFFF)
    
    # Head
    M5.Lcd.fillCircle(x, y - 20, 12, 0xFFFFFF)
    
    # Eyes
    M5.Lcd.fillCircle(x - 5, y - 23, 2, 0x000000)
    M5.Lcd.fillCircle(x + 5, y - 23, 2, 0x000000)
    
    # Ears
    M5.Lcd.fillCircle(x - 10, y - 28, 5, 0xFFFFFF)
    M5.Lcd.fillCircle(x + 10, y - 28, 5, 0xFFFFFF)
    
    # Legs
    M5.Lcd.fillRect(x - 12, y + 10, 4, 10, 0xFFFFFF)
    M5.Lcd.fillRect(x + 8, y + 10, 4, 10, 0xFFFFFF)
    
    # Tail (using drawLine instead of line)
    tail_angle = time.ticks_ms() / 200
    tail_x = x + int(10 * math.cos(tail_angle))
    tail_y = y + int(5 * math.sin(tail_angle))
    M5.Lcd.drawLine(x + 10, y, tail_x + 10, tail_y, 0xFFFFFF)

def draw_menu():
    """Draw the current menu on screen."""
    M5.Lcd.clear()
    M5.Lcd.setTextColor(0xFFFFFF)
    M5.Lcd.setTextSize(2)
    
    # Show menu title
    M5.Lcd.setCursor(10, 10)
    if in_sub_menu:
        M5.Lcd.print(f"{current_menu[selected_item]} Menu:")
    else:
        M5.Lcd.print("Main Menu:")
    
    # Show menu items
    for idx, item in enumerate(current_menu):
        color = 0xFF0000 if idx == selected_item else 0xFFFFFF
        M5.Lcd.setTextColor(color)
        M5.Lcd.setCursor(10, 40 + idx * 30)
        M5.Lcd.print(item)

def play_button_sound(button_type):
    """Play different sounds for different buttons."""
    if button_type == "select":  # Left button
        M5.Speaker.tone(440, 50)  # A4 note
        time.sleep_ms(50)
        M5.Speaker.tone(880, 50)  # A5 note
    elif button_type == "menu":   # Middle button
        M5.Speaker.tone(587, 50)  # D5 note
        time.sleep_ms(50)
        M5.Speaker.tone(440, 50)  # A4 note
        time.sleep_ms(50)
        M5.Speaker.tone(587, 50)  # D5 note
    elif button_type == "cancel": # Right button
        M5.Speaker.tone(880, 50)  # A5 note
        time.sleep_ms(50)
        M5.Speaker.tone(440, 50)  # A4 note

def update_pet_position():
    """Update pet's idle movement."""
    global pet_x, pet_y, move_direction, move_timer
    
    # Update position
    pet_x += move_direction
    
    # Check boundaries and reverse direction
    center_x = M5.Lcd.width() // 2
    if abs(pet_x - center_x) > move_range:
        move_direction = -move_direction
    
    # Add bobbing motion
    bob_height = 8  # Increased bobbing height
    pet_y = (M5.Lcd.height() // 2) + int(math.sin(time.ticks_ms() / 300) * bob_height)

# Main loop with reduced screen updates
last_update = time.ticks_ms()
UPDATE_INTERVAL = 100  # Slower updates (100ms instead of 50ms)

while True:
    M5.update()  # Update button states
    
    current_time = time.ticks_ms()
    if current_time - last_update >= UPDATE_INTERVAL:
        # Only clear the specific areas we're about to redraw
        if not menu_opened:
            # Clear sky area
            M5.Lcd.fillRect(0, 0, M5.Lcd.width(), M5.Lcd.height() // 2, 0x87CEEB)
            
            # Clear ground area
            M5.Lcd.fillRect(0, M5.Lcd.height() // 2, M5.Lcd.width(), M5.Lcd.height() // 2, 0x228B22)
            
            # Draw horizon line
            M5.Lcd.drawLine(0, M5.Lcd.height() // 2, M5.Lcd.width(), M5.Lcd.height() // 2, 0x000000)
            
            # Clear old pet position (small rectangle around previous position)
            old_x = pet_x
            old_y = pet_y
            M5.Lcd.fillRect(old_x - 20, old_y - 40, 40, 80, 
                           0x87CEEB if old_y < M5.Lcd.height() // 2 else 0x228B22)
            
            # Update and draw new content
            draw_sun_or_moon()
            display_stats()
            update_pet_position()
            draw_character(pet_x, pet_y)
        else:
            # Menu mode
            M5.Lcd.clear()
            draw_menu()
            handle_menu_navigation()
        
        last_update = current_time
    
    time.sleep(0.02)  # Slightly longer delay