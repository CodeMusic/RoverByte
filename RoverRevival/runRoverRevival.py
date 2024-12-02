import LCD_1in44
import time
from PIL import Image, ImageDraw, ImageFont, ImageColor
import random
import pygame
import os

class RoverPet:
    def __init__(self):
        self.happiness = 100
        self.hunger = 0
        self.energy = 100
        self.last_update = time.time()
        
        # Initialize display
        self.disp = LCD_1in44.LCD()
        Lcd_ScanDir = LCD_1in44.SCAN_DIR_DFT
        self.disp.LCD_Init(Lcd_ScanDir)
        self.disp.LCD_Clear()
        
        # Create blank image for drawing
        self.image = Image.new('RGB', (self.disp.width, self.disp.height))
        self.draw = ImageDraw.Draw(self.image)
        
        # Menu items for the three action buttons
        self.action_items = ['Feed', 'Play', 'Clean']
        
        # Rover art with LARGE expression
        self.rover_art = [
            "",
            "{exp}",
            ""
        ]
        
        # Add expressions dictionary
        self.expressions = {
            'sleeping': "(⇀‿‿↼)",
            'awakening': "(≖‿‿≖)",
            'normal': "(◕‿‿◕)",
            'observing': "( ⚆⚆)",
            'happy': "(•‿‿•)",
            'excited': "(ᵔ◡◡ᵔ)",
            'friendly': "(♥‿‿♥)",
            'motivated': "(☼‿‿☼)",
            'demotivated': "(≖__≖)",
            'bored': "(-__-)",
            'sad': "(╥☁╥ )",
            'lonely': "(ب__ب)"
        }
        
        self.current_expression = 'normal'
        self.last_expression_change = time.time()
        self.expression_update_interval = 30  # Change expression every 30 seconds when idle
        
        # Load fonts with smaller status font
        try:
            self.time_font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 12)
            self.expression_font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 48)  # Large for expression
            self.status_font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 8)  # Smaller for status
        except:
            try:
                self.time_font = ImageFont.truetype("/usr/share/fonts/TTF/DejaVuSans.ttf", 12)
                self.expression_font = ImageFont.truetype("/usr/share/fonts/TTF/DejaVuSans.ttf", 48)  # Large for expression
                self.status_font = ImageFont.truetype("/usr/share/fonts/TTF/DejaVuSans.ttf", 8)  # Smaller for status
            except:
                print("Warning: DejaVuSans font not found, using default font")
                self.time_font = ImageFont.load_default()
                self.expression_font = ImageFont.load_default()
                self.status_font = ImageFont.load_default()
        
        # Initialize pygame mixer for sounds
        pygame.mixer.init()
        
        # Load sound effects
        self.sounds = {
            # Rover sounds
            'bark': self.load_sound('bark.wav'),
            'whine': self.load_sound('whine.wav'),
            'happy': self.load_sound('happy.wav'),
            'sleeping': self.load_sound('sleeping.wav'),
            
            # Menu sounds
            'select': self.load_sound('select.wav'),
            'move': self.load_sound('move.wav')
        }
        
        # Sound cooldown
        self.last_sound_time = time.time()
        self.sound_cooldown = 2  # seconds between sounds
        
        # Status text format
        self.status_format = "Level: {level}  |  Mood: {mood}"
        
        # Add debug logging
        self.debug_log = []
        
        # Add button malfunction tracking
        self.malfunctioning_buttons = set()
    
    def load_sound(self, filename):
        try:
            sound_path = os.path.join(os.path.dirname(__file__), 'sounds', filename)
            return pygame.mixer.Sound(sound_path)
        except:
            print(f"Warning: Could not load sound {filename}")
            return None
    
    def play_sound(self, sound_name):
        current_time = time.time()
        if (current_time - self.last_sound_time) >= self.sound_cooldown:
            if sound_name in self.sounds and self.sounds[sound_name]:
                self.sounds[sound_name].play()
                self.last_sound_time = current_time
    
    def update_stats(self):
        current_time = time.time()
        time_passed = current_time - self.last_update
        
        # Update stats every minute
        if time_passed >= 60:
            self.hunger += 5
            self.energy -= 2
            self.happiness -= 3
            
            # Keep stats within bounds
            self.hunger = min(100, max(0, self.hunger))
            self.energy = min(100, max(0, self.energy))
            self.happiness = min(100, max(0, self.happiness))
            
            self.last_update = current_time

    def get_mood(self):
        # Determine mood based on stats
        if self.energy < 20:
            return 'sleeping'
        elif self.hunger > 80:
            return 'sad'
        elif self.happiness < 30:
            return 'lonely'
        elif self.happiness > 80:
            return 'happy'
        elif self.energy > 80:
            return 'motivated'
        
        # If no specific condition is met and it's time to change expression
        current_time = time.time()
        if current_time - self.last_expression_change > self.expression_update_interval:
            self.last_expression_change = current_time
            return random.choice(['normal', 'observing', 'friendly', 'excited'])
            
        return self.current_expression

    def draw_screen(self):
        # Clear screen
        self.draw.rectangle((0, 0, self.disp.width, self.disp.height), fill=0)
        
        # Draw time at the very top in 12-hour AM/PM format
        current_time = time.strftime("%I:%M:%S %p")
        time_bbox = self.time_font.getbbox(current_time)
        time_width = time_bbox[2] - time_bbox[0]
        self.draw.text((self.disp.width//2 - time_width//2, 2), 
                      current_time, fill="WHITE", font=self.time_font)
        
        # Update Rover's expression
        self.current_expression = self.get_mood()
        expression = self.expressions[self.current_expression]
        
        # Draw LARGE expression (centered, taking up most of the screen)
        exp_bbox = self.expression_font.getbbox(expression)
        exp_width = exp_bbox[2] - exp_bbox[0]
        exp_height = exp_bbox[3] - exp_bbox[1]
        
        # Calculate position to center the large expression
        exp_x = self.disp.width//2 - exp_width//2
        exp_y = self.disp.height//2 - exp_height//2
        
        # Draw the expression
        self.draw.text((exp_x, exp_y), expression, 
                      fill="WHITE", font=self.expression_font)
        
        # Draw status at bottom (smaller font)
        level = "1"
        status_text = self.status_format.format(level=level, mood=self.current_expression)
        status_bbox = self.status_font.getbbox(status_text)
        status_width = status_bbox[2] - status_bbox[0]
        self.draw.text((self.disp.width//2 - status_width//2, self.disp.height - 10), 
                      status_text, fill="WHITE", font=self.status_font)

    def log_button_press(self, button_name):
        self.debug_log.append(f"{time.strftime('%H:%M:%S')}: {button_name} pressed")
        print(f"Button pressed: {button_name}")  # Console debug output

    def check_button_malfunction(self, button_pin, button_name):
        """Check if a button appears to be stuck"""
        consecutive_reads = 0
        for _ in range(5):  # Check 5 times
            if self.disp.digital_read(button_pin) == 1:
                consecutive_reads += 1
            time.sleep(0.1)
        
        if consecutive_reads >= 4:  # Button appears stuck
            if button_name not in self.malfunctioning_buttons:
                self.malfunctioning_buttons.add(button_name)
                self.debug_log.append(f"WARNING: {button_name} appears to be malfunctioning")
                print(f"WARNING: {button_name} appears to be malfunctioning")
            return True
        return False

    def run(self):
        try:
            while True:
                self.update_stats()
                
                # Handle main action buttons (A and C only)
                if self.disp.digital_read(self.disp.GPIO_KEY1_PIN) == 1:  # A Button
                    if not self.check_button_malfunction(self.disp.GPIO_KEY1_PIN, "KEY1"):
                        self.handle_button_press(0)
                    time.sleep(0.2)
                
                if self.disp.digital_read(self.disp.GPIO_KEY3_PIN) == 1:  # C Button
                    if not self.check_button_malfunction(self.disp.GPIO_KEY3_PIN, "KEY3"):
                        self.handle_button_press(2)
                    time.sleep(0.2)
                
                # Handle directional buttons for affection
                if self.disp.digital_read(self.disp.GPIO_KEY_UP_PIN) == 1:
                    self.handle_button_press('UP')
                    time.sleep(0.2)
                
                if self.disp.digital_read(self.disp.GPIO_KEY_DOWN_PIN) == 1:
                    self.handle_button_press('DOWN')
                    time.sleep(0.2)
                
                if self.disp.digital_read(self.disp.GPIO_KEY_LEFT_PIN) == 1:
                    self.handle_button_press('LEFT')
                    time.sleep(0.2)
                
                if self.disp.digital_read(self.disp.GPIO_KEY_RIGHT_PIN) == 1:
                    self.handle_button_press('RIGHT')
                    time.sleep(0.2)
                
                # Update display
                self.draw_screen()
                self.disp.LCD_ShowImage(self.image, 0, 0)
                
        except Exception as e:
            print(f"Error: {e}")
            print("Debug log:")
            for log in self.debug_log[-10:]:
                print(log)
        finally:
            pygame.mixer.quit()
            self.disp.module_exit()

    def handle_button_press(self, button_num):
        """Handle button interactions with clear feedback"""
        original_expression = self.current_expression
        
        # Show immediate reaction
        if button_num == 0:  # KEY1 (A Button)
            self.current_expression = 'excited'
            self.play_sound('happy')
            self.happiness += 10
            self.log_button_press("Feed button - Rover is excited!")
            
        elif button_num == 2:  # KEY3 (C Button)
            self.current_expression = 'friendly'
            self.play_sound('bark')
            self.happiness += 5
            self.log_button_press("Clean button - Rover is friendly!")
            
        # Directional buttons for affection
        elif button_num == 'UP':
            self.current_expression = 'happy'
            self.play_sound('happy')
            self.log_button_press("Pat on head - Rover is happy!")
            
        elif button_num == 'DOWN':
            self.current_expression = 'friendly'
            self.play_sound('happy')
            self.log_button_press("Belly rub - Rover loves it!")
            
        elif button_num == 'LEFT':
            self.current_expression = 'excited'
            self.play_sound('bark')
            self.log_button_press("Scratch left ear - Rover is excited!")
            
        elif button_num == 'RIGHT':
            self.current_expression = 'excited'
            self.play_sound('bark')
            self.log_button_press("Scratch right ear - Rover is excited!")
        
        # Update display to show reaction
        self.draw_screen()
        self.disp.LCD_ShowImage(self.image, 0, 0)
        time.sleep(0.5)  # Show reaction
        
        # Return to previous expression
        self.current_expression = original_expression

if __name__ == '__main__':
    rover = RoverPet()
    rover.run()
