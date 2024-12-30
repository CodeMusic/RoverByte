import M5
from M5 import *
from hardware import *
import time

WHITE = 0xffffff
BLACK = 0x222222
NTAG215_PAGE_COUNT = 135

# Base frequencies for two octaves
NOTES = {
    'C6': 2093, 'C#6': 2217, 'D6': 2349, 'D#6': 2489, 'E6': 2637, 'F6': 2794, 'F#6': 2960,
    'G6': 3136, 'G#6': 3322, 'A6': 3520, 'A#6': 3729, 'B6': 3951,
    'C7': 4186, 'C#7': 4435, 'D7': 4699, 'D#7': 4978, 'E7': 5274, 'F7': 5588, 'F#7': 5920,
    'G7': 6272, 'G#7': 6645, 'A7': 7040, 'A#7': 7459, 'B7': 7902, 'C8': 8372
}

# Define major scales (indices from chromatic scale)
MAJOR_SCALES = {
    'C':  [0, 2, 4, 5, 7, 9, 11, 12],    # C D E F G A B C
    'G':  [7, 9, 11, 0, 2, 4, 6, 7],     # G A B C D E F# G
    'D':  [2, 4, 6, 7, 9, 11, 1, 2],     # D E F# G A B C# D
    'A':  [9, 11, 1, 2, 4, 6, 8, 9],     # A B C# D E F# G# A
    'E':  [4, 6, 8, 9, 11, 1, 3, 4],     # E F# G# A B C# D# E
    'F':  [5, 7, 9, 10, 0, 2, 4, 5],     # F G A Bb C D E F
    'Bb': [10, 0, 2, 3, 5, 7, 9, 10]     # Bb C D Eb F G A Bb
}

STATE_HOME = 0
STATE_READING = 1

def get_key_from_uid(uid_bytes):
    # Use first byte of UID to select key
    keys = list(MAJOR_SCALES.keys())
    key_index = uid_bytes[0] % len(keys)
    return keys[key_index]

def text_to_melody(text, uid_bytes=None):
    if not uid_bytes:
        key = 'C'  # Default to C major
    else:
        key = get_key_from_uid(uid_bytes)
    
    print(f"Playing in key of {key}")
    
    # Get the scale for our key
    scale = MAJOR_SCALES[key]
    notes = list(NOTES.values())
    
    # Create melody in the selected key
    melody = []
    chars = list(text.lower())
    i = 0
    while i < len(chars)-1:  # Process in pairs
        # First char determines pitch (or rest)
        if chars[i] == '.':  # Rest
            note = 0  # 0 frequency = silence
        else:
            scale_degree = ord(chars[i]) % 8
            chromatic_index = scale[scale_degree]
            octave_offset = 12 if ord(chars[i]) % 2 else 0
            note_index = chromatic_index + octave_offset
            note = notes[note_index]
        
        # Second char determines duration
        duration = 100 + (ord(chars[i+1]) % 400)  # Wider range of durations
        
        melody.append((note, duration))
        i += 2  # Move to next pair
    
    return melody

def play_melody(melody):
    for note, duration in melody:
        M5.update()  # Check for button press
        if M5.BtnA.wasPressed():
            return False  # Return False if button was pressed
        if note > 0:  # Only play if not a rest
            M5.Speaker.tone(note, duration)
        
        # Break up the sleep into smaller chunks to check button more frequently
        elapsed = 0
        chunk = 50  # Check every 50ms
        while elapsed < duration:
            M5.update()
            if M5.BtnA.wasPressed():
                return False
            time.sleep(min(chunk, duration - elapsed) / 1000)
            elapsed += chunk
            
        time.sleep(0.05)  # Small gap between notes
    return True  # Return True if melody completed

class RoverDisplay:
    def __init__(self):
        self.current_state = STATE_HOME
        self.svg_paths = {
            'base': {
                'head': [{'type': 'roundrect', 'coords': (60, 30, 100, 95, 25)}],
                'ears': [
                    {'type': 'triangle', 'coords': (60, 35, 40, 10, 60, 25)},
                    {'type': 'triangle', 'coords': (160, 35, 180, 10, 160, 25)}
                ],
                'eye_frame': [{'type': 'roundrect', 'coords': (75, 45, 70, 35, 12)}],
                'nose': [{'type': 'circle', 'coords': (105, 90, 8), 'color': BLACK}]
            },
            'expressions': {
                'home': {
                    'eyes': [
                        {'type': 'circle', 'coords': (90, 60, 12), 'color': WHITE},
                        {'type': 'circle', 'coords': (90, 60, 6), 'color': BLACK},
                        {'type': 'circle', 'coords': (130, 60, 12), 'color': WHITE},
                        {'type': 'circle', 'coords': (130, 60, 6), 'color': BLACK}
                    ],
                    'mouth': [{'type': 'curve', 'coords': (85, 105, 105, 110, 125, 105)}]
                },
                'reading': {
                    'eyes': [
                        {'type': 'circle', 'coords': (90, 60, 12), 'color': WHITE},
                        {'type': 'circle', 'coords': (90, 60, 6), 'color': BLACK},
                        {'type': 'circle', 'coords': (130, 60, 12), 'color': WHITE},
                        {'type': 'circle', 'coords': (130, 60, 6), 'color': BLACK}
                    ],
                    'mouth': [{'type': 'line', 'coords': (90, 105, 120, 105)}]
                },
                'failure': {
                    'eyes': [
                        {'type': 'circle', 'coords': (90, 60, 12), 'color': WHITE},
                        {'type': 'circle', 'coords': (90, 60, 6), 'color': BLACK},
                        {'type': 'circle', 'coords': (130, 60, 12), 'color': WHITE},
                        {'type': 'circle', 'coords': (130, 60, 6), 'color': BLACK}
                    ],
                    'mouth': [{'type': 'curve', 'coords': (85, 105, 105, 100, 125, 105)}]
                }
            }
        }

    def draw_text(self, text, x, y, scale=2):
        M5.Lcd.setTextSize(scale)
        width = len(text) * 6 * scale
        M5.Lcd.drawString(text, x - width//2, y)

    def draw_path(self, path, y_offset=0, color=WHITE):
        if path['type'] == 'roundrect':
            x, y, w, h, r = path['coords']
            M5.Lcd.fillRoundRect(x, y+y_offset, w, h, r, path.get('color', color))
        elif path['type'] == 'triangle':
            x1, y1, x2, y2, x3, y3 = path['coords']
            M5.Lcd.fillTriangle(x1, y1+y_offset, x2, y2+y_offset, x3, y3+y_offset, color)
        elif path['type'] == 'circle':
            x, y, r = path['coords']
            M5.Lcd.fillCircle(x, y+y_offset, r, path.get('color', color))
        elif path['type'] == 'line':
            x1, y1, x2, y2 = path['coords']
            M5.Lcd.drawLine(x1, y1+y_offset, x2, y2+y_offset, color)
        elif path['type'] == 'curve':
            x1, y1, x2, y2, x3, y3 = path['coords']
            M5.Lcd.drawLine(x1, y1+y_offset, x2, y2+y_offset, color)
            M5.Lcd.drawLine(x2, y2+y_offset, x3, y3+y_offset, color)

    def draw_rover(self, expression_key, y=60):
        M5.Lcd.fillRect(20, y, 160, 120, BLACK)
        for part in self.svg_paths['base']:
            for path in self.svg_paths['base'][part]:
                self.draw_path(path, y)
        expr = self.svg_paths['expressions'][expression_key]
        for feature in expr:
            for path in expr[feature]:
                self.draw_path(path, y, path.get('color', BLACK))

    def show_home(self):
        M5.Lcd.clear(BLACK)
        self.draw_rover('home')
        self.draw_text("Tap NTAG215", 160, 170, 1)

    def show_error(self, message="Read Failed"):
        M5.Lcd.clear(BLACK)
        self.draw_rover('failure')
        self.draw_text(message, 160, 200, 2)

def read_ntag215():
    try:
        if not rfid.is_new_card_present():
            return None
            
        if not rfid.picc_read_card_serial():
            return None

        uid_bytes = list(rfid._uid)  # Get UID early
        raw_content = ""
        buffer = bytearray(16)
        empty_page_count = 0  # Track consecutive empty pages
        
        for page in range(4, NTAG215_PAGE_COUNT):
            try:
                # Try up to 3 times to read each page
                for attempt in range(3):
                    status = rfid.mifare_read(page, buffer)
                    if status == rfid.STATUS_OK:
                        break
                    if attempt == 2:  # Only print on final retry
                        print(f"Read failed at page {page}")
                    time.sleep(0.01)
                
                if status != rfid.STATUS_OK:
                    continue
                    
                # Convert only the first 4 bytes to text
                text = ''.join(chr(b) for b in buffer[:4] if b >= 32 and b <= 126)
                if text:
                    raw_content += text
                    empty_page_count = 0
                else:
                    empty_page_count += 1
                    if empty_page_count >= 3:
                        break
                    
            except OSError as e:
                if e.errno == 19:  # ENODEV
                    print("RFID device lost, reinitializing...")
                    try:
                        rfid.pcd_init()
                        rfid.pcd_antenna_on()
                        time.sleep(0.1)
                        continue
                    except:
                        print("Recovery failed")
                        return None
                raise
                
        # Clean up and output final content
        if raw_content:
            content = raw_content.strip(': \t\n\r')
            content = ' '.join(content.split())
            print("Content:", content)
            return content, uid_bytes
            
        return None
        
    except Exception as e:
        print(f"NTAG read error: {str(e)}")
        return None

def setup():
    M5.begin()
    M5.Lcd.setRotation(0)
    M5.Lcd.setBrightness(100)
    global rover_display, rfid
    rover_display = RoverDisplay()
    rfid = RFID()
    rfid.pcd_init()
    rfid.pcd_antenna_on()
    M5.Speaker.setVolume(72)
    rover_display.show_home()

def reinit_rfid():
    global rfid
    rfid = RFID()
    rfid.pcd_init()
    rfid.pcd_antenna_on()
    time.sleep(0.2)

def loop():
    M5.update()  # Always update at start of loop
    
    # Always check button in READING state first
    if rover_display.current_state == STATE_READING:
        if M5.BtnA.wasPressed():
            M5.Speaker.tone(NOTES['C7'], 100)
            time.sleep(0.1)
            M5.Speaker.tone(NOTES['G6'], 100)
            time.sleep(0.1)
            M5.Speaker.tone(NOTES['E6'], 100)
            rover_display.current_state = STATE_HOME
            rover_display.show_home()
            return  # Exit loop after handling button
    
    # HOME state handling
    if rover_display.current_state == STATE_HOME:
        try:
            if rfid.is_new_card_present():
                M5.Speaker.tone(NOTES['E6'], 100)
                time.sleep(0.1)
                M5.Speaker.tone(NOTES['G6'], 100)
                content = read_ntag215()
                if content:
                    text, uid = content
                    rover_display.current_state = STATE_READING
                    M5.Lcd.clear(BLACK)
                    rover_display.draw_rover('reading', y=20)
                    
                    # Text display setup
                    M5.Lcd.setTextSize(1)
                    words = text.split()
                    line = ""
                    lines = []
                    
                    for word in words:
                        test_line = line + word + " "
                        if len(test_line) > 53:
                            lines.append(line.strip())
                            line = word + " "
                        else:
                            line = test_line
                    if line:
                        lines.append(line.strip())
                    
                    total_height = len(lines) * 15
                    start_y = max(120, (320 - total_height) // 2)
                    
                    for line in lines:
                        M5.Lcd.drawString(line, 10, start_y)
                        start_y += 15
                        M5.update()  # Check for button during text display
                    
                    melody = text_to_melody(text, uid)
                    if not play_melody(melody):
                        rover_display.current_state = STATE_HOME
                        rover_display.show_home()
                    M5.update()  # Make sure to update after melody
                else:
                    M5.Speaker.tone(NOTES['G6'], 200)
                    time.sleep(0.2)
                    M5.Speaker.tone(NOTES['E6'], 200)
                    rover_display.show_error("No NTAG data")
                    time.sleep(2)
                    rover_display.show_home()
        except OSError as e:
            if e.errno == 19:
                print("RFID device lost in main loop, reinitializing...")
                try:
                    reinit_rfid()
                except:
                    print("Main loop reinitialization failed")
                rover_display.show_error("RFID Reset")
                time.sleep(1)
                rover_display.show_home()
    
    M5.update()  # Always update at end of loop

if __name__ == '__main__':
    try:
        setup()
        while True:
            loop()
    except Exception as e:
        print(f"Error: {str(e)}")
