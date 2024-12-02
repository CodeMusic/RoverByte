import LCD_1in44
import time
from PIL import Image, ImageDraw, ImageFont, ImageColor

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
        
        # Menu state
        self.menu_items = ['Status', 'Feed', 'Play', 'Sleep']
        self.current_menu = 0

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

    def draw_menu(self):
        # Clear screen
        self.draw.rectangle((0, 0, self.disp.width, self.disp.height), fill=0)
        
        # Draw menu items
        y_pos = 20
        for i, item in enumerate(self.menu_items):
            if i == self.current_menu:
                self.draw.text((10, y_pos), f'> {item}', fill="WHITE")
            else:
                self.draw.text((10, y_pos), f'  {item}', fill="WHITE")
            y_pos += 20

    def draw_status(self):
        self.draw.rectangle((0, 0, self.disp.width, self.disp.height), fill=0)
        self.draw.text((10, 10), f'Happiness: {self.happiness}', fill="WHITE")
        self.draw.text((10, 30), f'Hunger: {self.hunger}', fill="WHITE")
        self.draw.text((10, 50), f'Energy: {self.energy}', fill="WHITE")

    def run(self):
        try:
            while True:
                self.update_stats()
                
                # Handle button inputs
                if self.disp.digital_read(self.disp.GPIO_KEY_UP_PIN) == 1:
                    self.current_menu = (self.current_menu - 1) % len(self.menu_items)
                    time.sleep(0.2)  # Debounce
                
                if self.disp.digital_read(self.disp.GPIO_KEY_DOWN_PIN) == 1:
                    self.current_menu = (self.current_menu + 1) % len(self.menu_items)
                    time.sleep(0.2)  # Debounce
                
                if self.disp.digital_read(self.disp.GPIO_KEY_PRESS_PIN) == 1:
                    self.handle_selection()
                    time.sleep(0.2)  # Debounce
                
                # Update display
                self.draw_menu()
                self.disp.LCD_ShowImage(self.image, 0, 0)
                
        except Exception as e:
            print(f"Error: {e}")
        finally:
            self.disp.module_exit()

    def handle_selection(self):
        if self.menu_items[self.current_menu] == 'Status':
            self.draw_status()
        elif self.menu_items[self.current_menu] == 'Feed':
            self.hunger = max(0, self.hunger - 20)
            self.happiness += 5
        elif self.menu_items[self.current_menu] == 'Play':
            self.happiness += 15
            self.energy -= 10
            self.hunger += 5
        elif self.menu_items[self.current_menu] == 'Sleep':
            self.energy = min(100, self.energy + 30)
            self.hunger += 10

if __name__ == '__main__':
    rover = RoverPet()
    rover.run()
