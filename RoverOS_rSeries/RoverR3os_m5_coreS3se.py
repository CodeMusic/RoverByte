import os, sys, io
import M5
from M5 import *
from hardware import *
import time
from module import LoraModule
import gc  # For garbage collection

# Global variables
lora433_0 = None
lora433_data = None
temp_file = '/flash/temp_audio.raw'

# UI states
MOOD_HAPPY = "Happy"
MOOD_LISTENING = "Listening"
MOOD_RECORDING = "Recording"
MOOD_PLAYING = "Playing"
current_mood = MOOD_HAPPY
current_level = 0

# UI elements
time_label = None
status_label = None

def draw_rover(mood, x=110, y=80):
    # Colors
    WHITE = 0xFFFFFF
    BLACK = 0x000000
    PINK = 0xFF9999
    
    # Base circle for head (white)
    M5.Lcd.fillCircle(x+50, y+50, 40, WHITE)
    
    # Eyes
    if mood == MOOD_HAPPY:
        # Happy eyes (^ ^)
        M5.Lcd.drawLine(x+30, y+40, x+40, y+30, BLACK)  # Left eye up
        M5.Lcd.drawLine(x+40, y+30, x+50, y+40, BLACK)  # Left eye down
        M5.Lcd.drawLine(x+60, y+40, x+70, y+30, BLACK)  # Right eye up
        M5.Lcd.drawLine(x+70, y+30, x+80, y+40, BLACK)  # Right eye down
    elif mood == MOOD_LISTENING:
        # Round attentive eyes (O O)
        M5.Lcd.fillCircle(x+40, y+40, 8, BLACK)  # Left eye
        M5.Lcd.fillCircle(x+70, y+40, 8, BLACK)  # Right eye
        M5.Lcd.fillCircle(x+42, y+38, 3, WHITE)  # Left eye highlight
        M5.Lcd.fillCircle(x+72, y+38, 3, WHITE)  # Right eye highlight
    elif mood == MOOD_RECORDING:
        # Wide eyes (O O)
        M5.Lcd.fillCircle(x+40, y+40, 10, BLACK)  # Left eye
        M5.Lcd.fillCircle(x+70, y+40, 10, BLACK)  # Right eye
        M5.Lcd.fillCircle(x+42, y+38, 4, WHITE)  # Left eye highlight
        M5.Lcd.fillCircle(x+72, y+38, 4, WHITE)  # Right eye highlight
    elif mood == MOOD_PLAYING:
        # Happy squinting eyes (^ ^)
        M5.Lcd.drawLine(x+30, y+35, x+50, y+35, BLACK)  # Left eye
        M5.Lcd.drawLine(x+60, y+35, x+80, y+35, BLACK)  # Right eye
        M5.Lcd.drawLine(x+35, y+30, x+45, y+35, BLACK)  # Left eye angle
        M5.Lcd.drawLine(x+65, y+30, x+75, y+35, BLACK)  # Right eye angle
    
    # Nose (always black)
    M5.Lcd.fillCircle(x+55, y+60, 8, BLACK)
    M5.Lcd.fillCircle(x+57, y+58, 3, PINK)  # Nose highlight
    
    # Mouth (changes with mood)
    if mood == MOOD_HAPPY or mood == MOOD_PLAYING:
        # Happy smile
        M5.Lcd.drawArc(x+55, y+60, 25, 20, 200, 340, BLACK)
    elif mood == MOOD_LISTENING:
        # Small "o"
        M5.Lcd.drawCircle(x+55, y+75, 5, BLACK)
    elif mood == MOOD_RECORDING:
        # Wide "O"
        M5.Lcd.drawCircle(x+55, y+75, 8, BLACK)
    
    # Ears (always the same)
    M5.Lcd.fillTriangle(x+20, y+20, x+40, y+10, x+30, y+40, WHITE)  # Left ear
    M5.Lcd.fillTriangle(x+80, y+20, x+60, y+10, x+70, y+40, WHITE)  # Right ear
    M5.Lcd.drawTriangle(x+20, y+20, x+40, y+10, x+30, y+40, BLACK)  # Left ear outline
    M5.Lcd.drawTriangle(x+80, y+20, x+60, y+10, x+70, y+40, BLACK)  # Right ear outline

def update_display(mood, level):
    # Clear screen with dark background
    M5.Lcd.fillScreen(0x222222)
    
    # Reset LCD properties
    M5.Lcd.setRotation(1)  # Standard rotation (portrait)
    M5.Lcd.setTextColor(0xFFFFFF)
    M5.Lcd.setBrightness(100)
    
    # Update time with larger font
    current_time = time.localtime()
    time_str = "{:02d}:{:02d}".format(current_time[3], current_time[4])
    M5.Lcd.setTextSize(3)  # Make time text larger
    M5.Lcd.setTextColor(0xFFFFFF)
    M5.Lcd.drawString(time_str, 110, 20)  # Moved down slightly and centered
    
    # Reset text size for Rover and status
    M5.Lcd.setTextSize(1)
    
    # Draw Rover with current mood
    draw_rover(mood)
    
    # Update status text
    status_text = f"Mood: {mood}, Level: {level}"
    M5.Lcd.drawString(status_text, 80, 200)  # Below Rover

def btnPWR_wasClicked_event(state):
    global lora433_0, current_mood
    if lora433_0:
        print("Starting LoRa receive mode...")
        lora433_0.start_recv()
        current_mood = MOOD_LISTENING
        update_display(current_mood, current_level)
    else:
        print("LoRa not available")
        M5.Lcd.fillScreen(0xff0000)  # Red flash
        time.sleep(1)
        update_display(current_mood, current_level)

def lora433_0_receive_event(received_data):
    global lora433_data, current_mood, current_level
    print("Data received!")
    lora433_data = received_data
    if lora433_data:
        current_mood = MOOD_PLAYING
        current_level += 1
        update_display(current_mood, current_level)
        Speaker.playWav(lora433_data)
        current_mood = MOOD_HAPPY
        update_display(current_mood, current_level)
    else:
        update_display(current_mood, current_level)

def record_sound():
    global current_mood
    print("Recording...")
    current_mood = MOOD_RECORDING
    update_display(current_mood, current_level)
    
    try:
        rec_data = bytearray(16000 * 3)
        Mic.begin()  # Make sure mic is initialized
        Mic.record(rec_data, 16000)
        
        while Mic.isRecording():
            M5.update()
            update_display(current_mood, current_level)  # Keep updating display
            
            if not (BtnA.isPressed() or BtnB.isPressed() or BtnC.isPressed()):
                print("Stopping recording...")
                Mic.end()
                break
            time.sleep_ms(10)
        
        print("Saving recording...")
        with open(temp_file, 'wb') as file:
            file.write(rec_data)
        print("Recording saved")
        return temp_file
        
    except Exception as e:
        print("Recording error:", e)
        return None
    finally:
        try:
            Mic.end()
        except:
            pass
        print("Returning to happy mood")
        current_mood = MOOD_HAPPY
        update_display(current_mood, current_level)

def play_sound(filename):
    global current_mood
    if not filename:
        print("No file to play")
        current_mood = MOOD_HAPPY
        update_display(current_mood, current_level)
        return
        
    print("Playing...")
    try:
        # Set mood and update before starting playback
        current_mood = MOOD_PLAYING
        update_display(current_mood, current_level)
        M5.update()
        
        Speaker.begin()
        Speaker.setVolume(1)
        
        with open(filename, 'rb') as file:
            data = file.read()
            print("Starting playback...")
            Speaker.playRaw(data, 16000)
            
            # Keep updating display while playing
            while Speaker.isPlaying():
                M5.update()
                update_display(current_mood, current_level)
                time.sleep_ms(50)  # Shorter sleep time for more frequent updates
                print("Still playing... updating display")
                
    except Exception as e:
        print("Playback error:", e)
    finally:
        try:
            Speaker.end()
        except:
            pass
        try:
            # Force a display reset
            M5.Lcd.fillScreen(0x222222)
            M5.Lcd.setTextSize(1)
            M5.Lcd.setTextColor(0xFFFFFF)
            M5.update()
            time.sleep_ms(100)
        except:
            pass
        print("Playback complete, returning to happy mood")
        current_mood = MOOD_HAPPY
        update_display(current_mood, current_level)
        M5.update()
        time.sleep_ms(100)

def setup():
    global lora433_0
    
    M5.begin()
    # Initialize display settings
    M5.Lcd.setRotation(1)  # Standard rotation (portrait)
    M5.Lcd.setBrightness(100)
    M5.Lcd.setTextColor(0xFFFFFF)
    update_display(current_mood, current_level)
    
    BtnPWR.setCallback(type=BtnPWR.CB_TYPE.WAS_CLICKED, cb=btnPWR_wasClicked_event)
    
    try:
        print("Initializing LoRa...")
        # Changed bandwidth to 7 (125kHz)
        lora433_0 = LoraModule(0, 10, 5, None, 8, 7, 8, 12, 200)
        lora433_0.set_irq_callback(lora433_0_receive_event)
        print("LoRa initialized successfully!")
    except Exception as e:
        print("LoRa initialization failed:", e)
        print("Continuing without LoRa functionality")
        lora433_0 = None

def loop():
    global current_mood, current_level
    
    M5.update()
    
    try:
        if BtnA.wasPressed():  # Left button - LoRa scan
            btnPWR_wasClicked_event(None)
        
        elif BtnB.wasPressed():  # Middle button - Record
            print("Starting record sequence")
            audio_file = record_sound()
            # Skip playback for now
            current_mood = MOOD_HAPPY
            update_display(current_mood, current_level)
        
        elif BtnC.wasPressed():  # Right button - Reset level
            current_level = 0
            update_display(current_mood, current_level)
        
    except Exception as e:
        print("Loop error:", e)
        current_mood = MOOD_HAPPY
        update_display(current_mood, current_level)
    
    time.sleep_ms(10)  # Removed constant display update

# Run the program
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