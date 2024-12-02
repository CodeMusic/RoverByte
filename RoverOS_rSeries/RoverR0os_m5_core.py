import M5
from M5 import *
from machine import Pin, I2C
import time
import math
import os
import machine
from machine import I2S, Pin

# Constants
SILVER = 0xC0C0C0
LIGHT_SILVER = 0xDCDCDC
BLACK = 0x000000
WHITE = 0xFFFFFF

# Animation settings
base_y = 120
BOB_AMPLITUDE = 12
BOB_SPEED = 1500
last_blink = 0

def setup():
    global temp_file, mic, baseline_noise
    M5.begin()
    
    # Initialize display
    M5.Lcd.setBrightness(100)
    M5.Lcd.setTextColor(WHITE)
    M5.Lcd.setTextSize(1)
    
    try:
        # Initialize microphone with ADC
        mic = machine.ADC(34)  # Pin 34 for microphone
        mic.atten(machine.ADC.ATTN_6DB)  # Changed to 6DB for better voice range
        mic.width(machine.ADC.WIDTH_12BIT)
        
        # Calibrate noise level
        baseline_noise = 0
        for _ in range(100):
            baseline_noise += mic.read()
            time.sleep_ms(1)
        baseline_noise = baseline_noise // 100
        print(f"Baseline noise level: {baseline_noise}")
        
        # Initialize speaker
        Speaker.begin()
        Speaker.setVolumePercentage(40)  # Increased volume
        Speaker.end()
        
        temp_file = '/flash/temp_audio.raw'
        print("Setup completed successfully")
            
    except Exception as e:
        print("Setup error:", e)

def draw_rover_custom(y_pos=120, perked=False, mouth_open=False, glasses=None):
    # Main head - white and fluffy
    M5.Lcd.fillRoundRect(120, y_pos-40, 80, 80, 20, WHITE)
    
    # Single silver panel with equal padding
    panel_width = 57
    panel_start_x = 131
    M5.Lcd.fillRoundRect(panel_start_x, y_pos-20, panel_width, 20, 5, SILVER)
    
    # Eyes
    M5.Lcd.fillCircle(140, y_pos-10, 8, SILVER)
    M5.Lcd.fillCircle(140, y_pos-10, 6, WHITE)
    M5.Lcd.fillCircle(140, y_pos-10, 3, BLACK)
    
    M5.Lcd.fillCircle(180, y_pos-10, 8, SILVER)
    M5.Lcd.fillCircle(180, y_pos-10, 6, WHITE)
    M5.Lcd.fillCircle(180, y_pos-10, 3, BLACK)
    
    # Ears - different angles based on perked state
    if perked:
        M5.Lcd.fillTriangle(
            110, y_pos-40,
            120, y_pos-40,
            95, y_pos-80,
            WHITE
        )
        M5.Lcd.fillTriangle(
            200, y_pos-40,
            210, y_pos-40,
            225, y_pos-80,
            WHITE
        )
    else:
        M5.Lcd.fillTriangle(
            110, y_pos-40,
            120, y_pos-40,
            95, y_pos-70,
            WHITE
        )
        M5.Lcd.fillTriangle(
            200, y_pos-40,
            210, y_pos-40,
            225, y_pos-70,
            WHITE
        )
    
    # Square black nose
    M5.Lcd.fillRect(155, y_pos+5, 10, 10, BLACK)
    
    # Thicker nose lines
    for i in range(2):
        M5.Lcd.drawLine(159+i, y_pos+15, 159+i, y_pos+25, BLACK)
    
    # Mouth - changes based on mouth_open state
    if mouth_open:
        for i in range(2):
            M5.Lcd.drawLine(145, y_pos+25+i, 175, y_pos+25+i, BLACK)
            M5.Lcd.drawLine(145-i, y_pos+25, 140-i, y_pos+30, BLACK)
            M5.Lcd.drawLine(175+i, y_pos+25, 180+i, y_pos+30, BLACK)
    else:
        for i in range(2):
            M5.Lcd.drawLine(145, y_pos+25+i, 160, y_pos+28+i, BLACK)
            M5.Lcd.drawLine(160, y_pos+28+i, 175, y_pos+25+i, BLACK)
    
    # Reading glasses if specified
    if glasses == 'reading':
        M5.Lcd.drawCircle(140, y_pos-10, 12, WHITE)
        M5.Lcd.drawCircle(180, y_pos-10, 12, WHITE)
        M5.Lcd.drawLine(152, y_pos-10, 168, y_pos-10, WHITE)
    elif glasses == 'sun':
        M5.Lcd.drawCircle(140, y_pos-10, 12, BLACK)
        M5.Lcd.drawCircle(180, y_pos-10, 12, SILVER)
        M5.Lcd.drawLine(152, y_pos-10, 168, y_pos-10, BLACK)

def record_sound():
    M5.Lcd.setCursor(40, 210)
    M5.Lcd.print("Recording...")
    
    try:
        print("Starting recording...")
        samples = []
        start_time = time.ticks_ms()
        
        # Record while button is held
        while M5.BtnA.isPressed() or M5.BtnB.isPressed() or M5.BtnC.isPressed():
            # Read ADC with noise reduction
            raw_value = mic.read()
            
            # Apply noise gate and scaling
            if abs(raw_value - baseline_noise) > 100:  # Noise threshold
                # Scale to 0-255 with noise reduction
                value = int(((raw_value - baseline_noise) / 2048) * 255)
                # Ensure value stays in valid range
                value = max(0, min(255, value))
            else:
                value = 128  # Center point for silence
                
            samples.append(value)
            
            M5.update()
            time.sleep_us(100)  # 10KHz sample rate
        
        # Apply simple noise reduction and amplification
        processed_samples = []
        window_size = 3
        for i in range(len(samples)):
            # Moving average for noise reduction
            start_idx = max(0, i - window_size)
            end_idx = min(len(samples), i + window_size + 1)
            window = samples[start_idx:end_idx]
            avg = sum(window) / len(window)
            
            # Amplify differences from center
            diff_from_center = avg - 128
            amplified = 128 + (diff_from_center * 1.5)  # Amplify by 1.5x
            processed_samples.append(int(max(0, min(255, amplified))))
        
        # Save processed audio
        with open(temp_file, 'wb') as file:
            file.write(bytes(processed_samples))
        
        duration = (time.ticks_ms() - start_time) / 1000.0
        print(f"Recording complete. Duration: {duration:.2f}s")
        print(f"File size: {os.stat(temp_file)[6]} bytes")
        return temp_file
        
    except Exception as e:
        print("Recording error:", e)
        print("Error details:", str(e))
        return None

def play_sound(filename):
    if filename:
        M5.Lcd.setCursor(40, 210)
        M5.Lcd.print("Playing...")
        
        try:
            Speaker.begin()
            Speaker.setVolumePercentage(40)
            
            with open(filename, 'rb') as file:
                data = bytearray(file.read())
                
                # Additional audio processing before playback
                for i in range(len(data)):
                    # Center audio around 128 and amplify
                    centered = data[i] - 128
                    amplified = int(128 + (centered * 1.8))  # Amplify by 1.8x
                    data[i] = max(0, min(255, amplified))
                
                print(f"Playing file size: {len(data)} bytes")
                Speaker.playRaw(data, 10000)
                
                while Speaker.isPlaying():
                    time.sleep_ms(10)
            
            print("Playback complete")
            
        except Exception as e:
            print("Playback error:", e)
            print("Error details:", str(e))
        finally:
            Speaker.end()
        
        M5.Lcd.setCursor(40, 210)
        M5.Lcd.print("Done!")

# Main loop setup
setup()
last_y = 120
SINE_STEPS = 100
sine_table = [math.sin(2 * math.pi * i / SINE_STEPS) for i in range(SINE_STEPS)]
current_step = 0

# Main loop
while True:
    M5.update()
    current_time = time.ticks_ms()
    
    current_step = (current_step + 1) % SINE_STEPS
    y_offset = int(sine_table[current_step] * 12)
    new_y = 120 + y_offset
    
    if M5.BtnA.wasPressed():
        M5.Lcd.clear(BLACK)
        draw_rover_custom(new_y, perked=True, mouth_open=True)
        audio_data = record_sound()
        if audio_data:
            draw_rover_custom(new_y, perked=True, mouth_open=True)
            play_sound(audio_data)
    
    elif M5.BtnB.wasPressed():
        M5.Lcd.clear(BLACK)
        draw_rover_custom(new_y, perked=False, mouth_open=False)
        audio_data = record_sound()
        if audio_data:
            draw_rover_custom(new_y, perked=True, mouth_open=True)
            play_sound(audio_data)
    
    elif M5.BtnC.wasPressed():
        M5.Lcd.clear(BLACK)
        draw_rover_custom(new_y, perked=True, glasses='reading')
        audio_data = record_sound()
        if audio_data:
            draw_rover_custom(new_y, perked=True, mouth_open=True)
            play_sound(audio_data)
    
    elif abs(new_y - last_y) >= 1:
        M5.Lcd.clear(BLACK)
        draw_rover_custom(new_y)
        last_y = new_y
    
    time.sleep(0.02)