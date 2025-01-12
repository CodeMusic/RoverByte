import os, sys, io
import M5
from M5 import *
from hardware import *
import time
import json
from micropython import const

# Constants
WHITE = const(0xffffff)
BLACK = const(0x222222)
MONEY_INCREMENT = 0.50  # 50 cents
MOCK_MODE = False
MOCK_PRESS_COUNT = 0
MOCK_BALANCE = 27.50  # Starting mock balance

# Musical Notes (frequencies in Hz)
NOTE_C = const(2093)      # C7 (higher octave)
NOTE_D = const(2349)      # D7
NOTE_E = const(2637)      # E7
NOTE_F = const(2794)      # F7
NOTE_G = const(3136)      # G7
NOTE_A = const(3520)      # A7
NOTE_B = const(3951)      # B7
NOTE_C_HIGH = const(4186) # C8

# UI States
STATE_HOME = const(0)
STATE_READING = const(1)
STATE_BALANCE = const(2)
STATE_WRITE_READY = const(3)
STATE_WRITING = const(4)

# Add encoding/decoding functions
def crc16_ccitt(data: bytes, poly=0x1021, init_val=0xFFFF) -> int:
    crc = init_val
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ poly) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF
    return crc

def decode_field_value(field_type, hex_pairs):
    """Decodes hex pairs based on field type"""
    hex_pairs = [hp.upper() for hp in hex_pairs]
    if field_type == "money":
        combined_hex = "".join(reversed(hex_pairs))
        val = int(combined_hex, 16)
        return val / 100.0
    elif field_type == "int":
        combined_hex = "".join(reversed(hex_pairs))
        return int(combined_hex, 16)
    elif field_type == "uid":
        combined_hex = "".join(hex_pairs)
        return int(combined_hex, 16)
    elif field_type == "checksum":
        combined_hex = "".join(hex_pairs)
        return int(combined_hex, 16)
    return None

def encode_money(amount: float) -> list:
    """Encodes money amount into little endian hex bytes"""
    cents = int(round(amount * 100))
    hex_str = f"{cents:04X}"
    little_endian = [hex_str[i:i+2] for i in range(0, 4, 2)]
    little_endian.reverse()
    return little_endian

class RoverDisplay:
    def __init__(self):
        self.current_state = STATE_HOME
        self.balance = 0.0
        self.original_balance = 0.0
        self.card_data = {}
        self.write_pending = False
        
        # SVG-style paths for Rover
        self.svg_paths = {
            'base': {
                'head': [
                    # Rounder head shape
                    {'type': 'roundrect', 'coords': (60, 30, 100, 95, 25)},
                ],
                'ears': [
                    # Floppier ears (longer triangles)
                    {'type': 'triangle', 'coords': (60, 35, 40, 10, 60, 25)},
                    {'type': 'triangle', 'coords': (160, 35, 180, 10, 160, 25)}
                ],
                'eye_frame': [
                    # Softer eye frame
                    {'type': 'roundrect', 'coords': (75, 45, 70, 35, 12)}
                ],
                'nose': [
                    # Add a nose!
                    {'type': 'circle', 'coords': (105, 90, 8), 'color': BLACK}
                ]
            },
            'expressions': {
                'home': {
                    'eyes': [
                        # Bigger, rounder eyes
                        {'type': 'circle', 'coords': (90, 60, 12), 'color': BLACK},
                        {'type': 'circle', 'coords': (90, 60, 11), 'color': 0x001F},
                        {'type': 'circle', 'coords': (90, 60, 5), 'color': BLACK},
                        {'type': 'circle', 'coords': (120, 60, 12), 'color': BLACK},
                        {'type': 'circle', 'coords': (120, 60, 11), 'color': 0x001F},
                        {'type': 'circle', 'coords': (120, 60, 5), 'color': BLACK}
                    ],
                    'brows': [
                        # Expressive eyebrows
                        {'type': 'line', 'coords': (80, 45, 95, 43)},
                        {'type': 'line', 'coords': (115, 45, 130, 43)}
                    ],
                    'mouth': [
                        # Happy dog mouth
                        {'type': 'curve', 'coords': (85, 105, 105, 110, 125, 105)}
                    ]
                },
                'reading': {
                    'eyes': [
                        # Focused eyes
                        {'type': 'circle', 'coords': (90, 60, 12), 'color': BLACK},
                        {'type': 'circle', 'coords': (90, 60, 11), 'color': 0x001F},
                        {'type': 'circle', 'coords': (90, 60, 4), 'color': BLACK},
                        {'type': 'circle', 'coords': (120, 60, 12), 'color': BLACK},
                        {'type': 'circle', 'coords': (120, 60, 11), 'color': 0x001F},
                        {'type': 'circle', 'coords': (120, 60, 4), 'color': BLACK}
                    ],
                    'brows': [
                        # Concentrated brows
                        {'type': 'line', 'coords': (80, 40, 95, 45)},
                        {'type': 'line', 'coords': (115, 40, 130, 45)}
                    ],
                    'mouth': [
                        # Focused mouth
                        {'type': 'line', 'coords': (90, 105, 120, 105)}
                    ]
                },
                'failure': {
                    'eyes': [
                        # Sad eyes (droopy)
                        {'type': 'circle', 'coords': (90, 65, 12), 'color': BLACK},
                        {'type': 'circle', 'coords': (90, 65, 11), 'color': 0x001F},
                        {'type': 'circle', 'coords': (90, 65, 4), 'color': BLACK},
                        {'type': 'circle', 'coords': (120, 65, 12), 'color': BLACK},
                        {'type': 'circle', 'coords': (120, 65, 11), 'color': 0x001F},
                        {'type': 'circle', 'coords': (120, 65, 4), 'color': BLACK}
                    ],
                    'brows': [
                        # Sad eyebrows
                        {'type': 'line', 'coords': (80, 48, 95, 45)},
                        {'type': 'line', 'coords': (115, 48, 130, 45)}
                    ],
                    'mouth': [
                        # Sad mouth (inverted curve)
                        {'type': 'curve', 'coords': (85, 110, 105, 105, 125, 110)}
                    ]
                },
                'writing': {
                    'eyes': [
                        # Focused eyes
                        {'type': 'circle', 'coords': (90, 60, 12), 'color': BLACK},
                        {'type': 'circle', 'coords': (90, 60, 11), 'color': 0x001F},
                        {'type': 'circle', 'coords': (90, 60, 3), 'color': BLACK},
                        {'type': 'circle', 'coords': (120, 60, 12), 'color': BLACK},
                        {'type': 'circle', 'coords': (120, 60, 11), 'color': 0x001F},
                        {'type': 'circle', 'coords': (120, 60, 3), 'color': BLACK}
                    ],
                    'brows': [
                        # Concentrated brows
                        {'type': 'line', 'coords': (80, 42, 95, 45)},
                        {'type': 'line', 'coords': (115, 42, 130, 45)}
                    ],
                    'mouth': [
                        # Focused mouth
                        {'type': 'line', 'coords': (90, 105, 120, 105)}
                    ]
                }
            }
        }
        
        # Embed template directly
        self.template = {
            "fields": [
                {
                    "name": "UID",
                    "type": "uid",
                    "block": 0,
                    "offset": 0,
                    "length": 4
                },
                {
                    "name": "Balance",
                    "type": "money",
                    "occurrences": [
                        { "block": 4, "offset": 0, "length": 2 },
                        { "block": 4, "offset": 8, "length": 2 },
                        { "block": 8, "offset": 0, "length": 2 },
                        { "block": 8, "offset": 8, "length": 2 }
                    ]
                },
                {
                    "name": "Balance Checksum",
                    "type": "checksum",
                    "occurrences": [
                        { "block": 4, "offset": 4, "length": 2 },
                        { "block": 4, "offset": 12, "length": 2 },
                        { "block": 8, "offset": 4, "length": 2 },
                        { "block": 8, "offset": 12, "length": 2 }
                    ]
                },
                {
                    "name": "Last Top-Up",
                    "type": "money",
                    "occurrences": [
                        { "block": 2, "offset": 9, "length": 2 }
                    ]
                },
                {
                    "name": "Top-Up Count",
                    "type": "int",
                    "occurrences": [
                        { "block": 2, "offset": 5, "length": 2 },
                        { "block": 4, "offset": 2, "length": 2 },
                        { "block": 4, "offset": 10, "length": 2 },
                        { "block": 8, "offset": 2, "length": 2 },
                        { "block": 8, "offset": 10, "length": 2 }
                    ]
                },
                {
                    "name": "Card Usage Left",
                    "type": "int",
                    "occurrences": [
                        { "block": 9, "offset": 0, "length": 2 },
                        { "block": 9, "offset": 8, "length": 2 }
                    ]
                }
            ]
        }
        
    def draw_text(self, text, x, y, scale=2):
        """Helper to draw centered text - smaller scale"""
        M5.Lcd.setTextSize(scale)
        width = len(text) * 6 * scale  # Approximate width
        M5.Lcd.drawString(text, x - width//2, y)
        
    def draw_path(self, path, y_offset=0, color=WHITE):
        """Draw an SVG-style path"""
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
            M5.Lcd.drawLine(x1, y1+y_offset+1, x2, y2+y_offset+1, color)  # Double thickness
        elif path['type'] == 'curve':
            x1, y1, x2, y2, x3, y3 = path['coords']
            M5.Lcd.drawLine(x1, y1+y_offset, x2, y2+y_offset, color)
            M5.Lcd.drawLine(x2, y2+y_offset, x3, y3+y_offset, color)
            # Double thickness
            M5.Lcd.drawLine(x1, y1+y_offset+1, x2, y2+y_offset+1, color)
            M5.Lcd.drawLine(x2, y2+y_offset+1, x3, y3+y_offset+1, color)

    def draw_rover(self, expression_key, y=60):
        """Draw Rover using SVG paths"""
        M5.Lcd.fillRect(20, y, 160, 120, BLACK)  # Clear area
        
        # Draw base components
        for part in self.svg_paths['base']:
            for path in self.svg_paths['base'][part]:
                if part == 'eye_frame':
                    self.draw_path(path, y, 0x4208)  # Dark gray for metal frame
                else:
                    self.draw_path(path, y)
        
        # Draw expression components
        expr = self.svg_paths['expressions'][expression_key]
        for feature in expr:
            for path in expr[feature]:
                self.draw_path(path, y, path.get('color', BLACK))
        
    def show_home(self):
        M5.Lcd.clear(BLACK)
        self.draw_rover('home')
        self.draw_text("Rover is Sniffing", 160, 170, 1)  # Moved up, smaller
        self.draw_text("for RFID Cards", 160, 185, 1)    # Moved up, smaller
        
    def show_balance(self):
        """Show the balance screen with proper currency symbol"""
        M5.Lcd.clear(BLACK)
        self.draw_rover('writing' if self.write_pending else 'home')
        
        # Draw balance with proper £ symbol and better positioning
        M5.Lcd.setTextSize(3)
        balance_text = f"£{self.balance:.2f}"
        text_width = len(balance_text) * 18
        M5.Lcd.drawString(balance_text, 160 - text_width//2, 180)  # Moved up slightly
        
        if self.write_pending:
            self.draw_text("Press to Cancel", 160, 210, 1)  # Adjusted position
    
    def update_balance(self, change):
        """Update balance with increment/decrement"""
        new_balance = self.balance + change
        if new_balance >= 0:  # Prevent negative balance
            self.balance = new_balance
            self.show_balance()
            # Play tick sound for feedback
            M5.Speaker.tone(NOTE_C, 50)
    
    def show_reading(self):
        M5.Lcd.clear(BLACK)
        self.draw_rover('reading')
        self.draw_text("Reading Card...", 160, 200, 2)
        
    def show_error(self, message="Read Failed"):
        M5.Lcd.clear(BLACK)
        self.draw_rover('failure')
        self.draw_text(message, 160, 200, 2)
        
    def decode_card_data(self, block_data):
        """Decodes card data according to template"""
        result = {}
        for field in self.template['fields']:
            name = field['name']
            field_type = field['type']
            
            if 'occurrences' in field:
                values = []
                for occ in field['occurrences']:
                    block = block_data.get(occ['block'])
                    if block:
                        data = block[occ['offset']:occ['offset'] + occ['length']]
                        values.append(decode_field_value(field_type, data))
                result[name] = values[0] if values else None
            elif 'block' in field:
                # Single occurrence fields like UID
                block = block_data.get(field['block'])
                if block:
                    data = block[field['offset']:field['offset'] + field['length']]
                    result[name] = decode_field_value(field_type, data)
        return result
        
    def encode_card_data(self):
        """Encodes current balance and other fields for writing"""
        block_data = {}
        
        # Find Balance field in template
        balance_field = next((f for f in self.template['fields'] if f['name'] == 'Balance'), None)
        if balance_field and 'occurrences' in balance_field:
            # Encode current balance
            balance_bytes = encode_money(self.balance)
            
            # Write to all balance occurrences
            for occ in balance_field['occurrences']:
                block_num = occ['block']
                if block_num not in block_data:
                    block_data[block_num] = ['00'] * 16
                    
                # Write balance bytes
                for i, byte in enumerate(balance_bytes):
                    block_data[block_num][occ['offset'] + i] = byte
                    
                # Calculate and write checksum if needed
                checksum = calculate_checksum(balance_bytes)
                # Find corresponding checksum field
                checksum_field = next((f for f in self.template['fields'] 
                                     if f['name'] == 'Balance Checksum'), None)
                if checksum_field:
                    for c_occ in checksum_field['occurrences']:
                        if c_occ['block'] == block_num:
                            for i, byte in enumerate(checksum):
                                block_data[block_num][c_occ['offset'] + i] = byte
                                
        return block_data

# Add working keys from Flipper Zero
KNOWN_KEYS = [
    [0x07, 0x34, 0xBF, 0xB9, 0x3D, 0xAB],  # Known working key 1
    [0x85, 0xA4, 0x38, 0xF7, 0x2A, 0x8A],  # Known working key 2
    [0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF],  # Factory default key (backup)
]

def init_rfid():
    """Initialize RFID module"""
    print("Initializing RFID...")
    global rfid
    rfid = RFID()  # Create new instance
    rfid.pcd_init()
    time.sleep(0.1)
    rfid.pcd_antenna_on()
    time.sleep(0.1)
    print("RFID initialization complete")

def wait_for_card():
    """Wait for card and return UID if found"""
    try:
        # Basic card detection first - even in mock mode
        if rfid.is_new_card_present():
            if rfid.picc_read_card_serial():
                # Get UID as integer
                uid_list = list(rfid._uid)[:4]  # Take first 4 bytes only
                uid_int = int.from_bytes(bytes(uid_list), 'big')
                print(f"Card detected! UID: {uid_int}")
                
                if MOCK_MODE:
                    print("MOCK MODE: Using mock data")
                    return bytes([0x01, 0x02, 0x03, 0x04])  # Mock UID
                else:
                    return bytes(uid_list)
                    
        return None
            
    except Exception as e:
        print(f"Card detection error: {str(e)}")
        return None

def setup():
    global rover_display, rfid, speaker
    M5.begin()
    M5.Lcd.setRotation(0)  # Ensure correct orientation for UIFlow2
    M5.Lcd.setBrightness(100)  # Full brightness
    
    rover_display = RoverDisplay()
    init_rfid()
    speaker = Speaker
    
    # Set initial speaker volume
    M5.Speaker.setVolume(100)
    
    rover_display.show_home()

def loop():
    M5.update()
    
    # Add continuous card checking when in HOME state
    if rover_display.current_state == STATE_HOME:
        uid = wait_for_card()
        if uid:
            read_card()
    
    # Handle balance adjustments
    if rover_display.current_state == STATE_BALANCE:
        encoder_diff = 0
        
        # Check for physical dial first
        if hasattr(M5, 'Dial'):
            encoder_diff = M5.Dial.getCount()
            if encoder_diff != 0:
                print(f"Dial turned: {encoder_diff}")
                M5.Dial.setCount(0)
        
        # Fall back to buttons if no dial or dial didn't move
        if encoder_diff == 0:
            if BtnB.wasPressed():
                print("BtnB pressed - incrementing")
                encoder_diff = 1
            elif BtnC.wasPressed():
                print("BtnC pressed - decrementing")
                encoder_diff = -1
        
        # Update balance if we have a change
        if encoder_diff != 0:
            new_balance = rover_display.balance + (encoder_diff * MONEY_INCREMENT)
            if new_balance >= 0:  # Prevent negative balance
                rover_display.balance = new_balance
                rover_display.show_balance()
                # Click feedback
                M5.Speaker.tone(NOTE_C if encoder_diff > 0 else NOTE_E, 50)

def try_read_block(block_num, uid):
    """Try to read a specific block using Mifare methods"""
    try:
        print(f"\nTrying to read block {block_num}")
        
        # Try each known key
        for key_index, key in enumerate(KNOWN_KEYS, 1):
            print(f"\nTrying key {key_index}: {bytes(key).hex()}")
            
            # Select card first
            if not rfid.picc_select_card():
                print("Card select failed")
                rfid.pcd_stop_crypto1()
                continue
                
            # Try to authenticate with key A
            status = rfid.pcd_authenticate(rfid.PICC_CMD_MF_AUTH_KEY_A, block_num, key, uid)
            if status != rfid.STATUS_OK:
                print(f"Auth A failed with key {key_index}")
                rfid.pcd_stop_crypto1()
                
                # Try key B if key A fails
                status = rfid.pcd_authenticate(rfid.PICC_CMD_MF_AUTH_KEY_B, block_num, key, uid)
                if status != rfid.STATUS_OK:
                    print(f"Auth B failed with key {key_index}")
                    rfid.pcd_stop_crypto1()
                    continue
                    
            print(f"Auth success with key {key_index}")
            
            # Try to read
            status, data = rfid.mifare_read(block_num)
            if status == rfid.STATUS_OK:
                print(f"Read success: {bytes(data).hex()}")
                return bytes(data)
            else:
                print(f"Read failed: {rfid.get_status_code_name(status)}")
                
            rfid.pcd_stop_crypto1()
                
        print("All keys failed")
        return None
        
    except Exception as e:
        print(f"Read error: {str(e)}")
        return None
    finally:
        rfid.pcd_stop_crypto1()

def toggle_mock_mode():
    global MOCK_MODE, MOCK_PRESS_COUNT
    MOCK_PRESS_COUNT += 1
    if MOCK_PRESS_COUNT >= 4:
        MOCK_MODE = not MOCK_MODE
        MOCK_PRESS_COUNT = 0
        # Play special tune for mock mode toggle
        if MOCK_MODE:
            # Happy ascending tune
            M5.Speaker.tone(NOTE_C, 100)
            time.sleep(0.1)
            M5.Speaker.tone(NOTE_E, 100)
            time.sleep(0.1)
            M5.Speaker.tone(NOTE_G, 100)
            time.sleep(0.1)
            M5.Speaker.tone(NOTE_C_HIGH, 200)
        else:
            # Descending tune for mock mode off
            M5.Speaker.tone(NOTE_C_HIGH, 100)
            time.sleep(0.1)
            M5.Speaker.tone(NOTE_G, 100)
            time.sleep(0.1)
            M5.Speaker.tone(NOTE_E, 100)
            time.sleep(0.1)
            M5.Speaker.tone(NOTE_C, 200)
        return True
    return False

def read_card():
    """Read card with LOUD musical feedback"""
    rover_display.current_state = STATE_READING
    M5.Lcd.clear(BLACK)
    rover_display.draw_rover('reading')
    rover_display.draw_text("Reading Card...", 160, 200, 2)
    
    # Sniffing tune (playful searching sound)
    M5.Speaker.setVolume(100)  # Max volume
    M5.Speaker.tone(NOTE_E, 100)  # E
    time.sleep(0.1)
    M5.Speaker.tone(NOTE_G, 100)  # G
    time.sleep(0.1)
    M5.Speaker.tone(NOTE_A, 100)  # A
    
    try:
        uid = wait_for_card()
        if not uid:
            raise Exception("Could not detect card")
            
        uid_int = int.from_bytes(uid, 'big')
        print(f"\nReading card with UID: {uid_int}")
        
        if MOCK_MODE:
            print("MOCK MODE: Using mock balance")
            rover_display.card_data = {'Balance': MOCK_BALANCE}
            rover_display.balance = MOCK_BALANCE
            rover_display.original_balance = MOCK_BALANCE
            rover_display.current_state = STATE_BALANCE
            rover_display.show_balance()
            return
            
        # Real card reading logic
        print("\nReading manufacturer block (0)...")
        manuf_block = try_read_block(0, uid)
        if not manuf_block:
            print("\nTrying block 1...")
            manuf_block = try_read_block(1, uid)
            if not manuf_block:
                raise Exception("Auth Failed")
            
            # Try reading balance block
            print("\nReading balance block...")
            data = try_read_block(8, uid)
            if not data:
                raise Exception("Auth Failed")
                
            rover_display.card_data = decode_card_data(data)
            rover_display.balance = rover_display.card_data.get('Balance', 0.0)
            rover_display.original_balance = rover_display.balance
            
        # Success tune (happy bark melody)
        M5.Speaker.tone(NOTE_C, 100)
        time.sleep(0.1)
        M5.Speaker.tone(NOTE_E, 100)
        time.sleep(0.1)
        M5.Speaker.tone(NOTE_G, 100)
        time.sleep(0.1)
        M5.Speaker.tone(NOTE_C_HIGH, 200)
        
        rover_display.current_state = STATE_BALANCE
        rover_display.show_balance()
        
    except Exception as e:
        print(f"Card read error: {str(e)}")
        M5.Lcd.clear(BLACK)
        rover_display.draw_rover('failure')
        error_msg = str(e) if len(str(e)) < 20 else "Read Failed"
        rover_display.draw_text(error_msg, 160, 200, 2)
        
        # Error tune (sad whimper)
        M5.Speaker.tone(NOTE_G, 200)
        time.sleep(0.2)
        M5.Speaker.tone(NOTE_E, 200)
        time.sleep(0.2)
        M5.Speaker.tone(NOTE_C, 300)
        
        time.sleep(2)
        rover_display.current_state = STATE_HOME
        rover_display.show_home()


def write_card_data():
    """Write updated card data back to card"""
    try:
        if MOCK_MODE:
            print("MOCK MODE: Simulating successful write")
            # Success tune (excited bark)
            M5.Speaker.tone(NOTE_C, 100)
            time.sleep(0.1)
            M5.Speaker.tone(NOTE_E, 100)
            time.sleep(0.1)
            M5.Speaker.tone(NOTE_G, 100)
            time.sleep(0.1)
            M5.Speaker.tone(NOTE_C_HIGH, 200)
            
            rover_display.draw_rover('success')
            rover_display.draw_text("Write Complete!", 160, 200, 2)
            time.sleep(2)
            return
            
        # Writing sound (processing tune)
        M5.Speaker.tone(NOTE_E, 100)
        time.sleep(0.1)
        M5.Speaker.tone(NOTE_G, 100)
        
        # Get encoded data
        block_data = rover_display.encode_card_data()
        
        # Write blocks using Mifare write
        for block_num, data in block_data.items():
            status = rfid.mifare_write(block_num, bytes([int(b, 16) for b in data]))
            if status != rfid.STATUS_OK:
                status_name = rfid.get_status_code_name(status)
                raise Exception(f"Failed to write block {block_num}. Status: {status_name}")
                
        # Success sound (happy bark)
        M5.Speaker.tone(NOTE_C, 100)
        time.sleep(0.1)
        M5.Speaker.tone(NOTE_E, 100)
        time.sleep(0.1)
        M5.Speaker.tone(NOTE_G, 100)
        time.sleep(0.1)
        M5.Speaker.tone(NOTE_C_HIGH, 200)
        
        rover_display.draw_rover('success')
        rover_display.draw_text("Write Complete!", 160, 200, 2)
        time.sleep(2)
        
    except Exception as e:
        print(f"Write error: {e}")
        rover_display.draw_rover('failure')
        rover_display.draw_text("Write Failed", 160, 200, 2)
        
        # Error sound (sad woof)
        M5.Speaker.tone(NOTE_G, 200)
        time.sleep(0.1)
        M5.Speaker.tone(NOTE_E, 200)
        time.sleep(0.1)
        M5.Speaker.tone(NOTE_C, 300)
        
        time.sleep(2)
    finally:
        rfid.pcd_stop_crypto1()  # Always stop crypto after operations



if __name__ == '__main__':
    try:
        setup()
        while True:
            loop()
    except (Exception, KeyboardInterrupt) as e:
        try:
            from utility import print_error_msg
            print_error_msg(e)
        except ImportError:
            print("please update to latest firmware")
