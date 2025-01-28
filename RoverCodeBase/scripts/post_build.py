Import("env")
import os

def after_build(source, target, env):
    # Verify FastLED configuration
    build_flags = env.get('BUILD_FLAGS', [])
    required_flags = [
        'FASTLED_ESP32_I2S',
        'FASTLED_RMT_MAX_CHANNELS',
        'FASTLED_ESP32_FLASH_LOCK',
        'FASTLED_ALL_PINS_HARDWARE_SPI'
    ]
    
    missing_flags = [flag for flag in required_flags if not any(flag in str(f) for f in build_flags)]
    if missing_flags:
        print("Warning: Missing FastLED flags:", missing_flags)

    # Check for Arduino libraries
    arduino_lib_dir = os.path.expanduser('~/Documents/Arduino/libraries')
    required_libs = ['FastLED', 'Adafruit_PN532', 'RotaryEncoder']
    
    if os.path.exists(arduino_lib_dir):
        missing_libs = [lib for lib in required_libs if not os.path.exists(os.path.join(arduino_lib_dir, lib))]
        if missing_libs:
            print("Warning: Missing Arduino libraries:", missing_libs)

env.AddPostAction("buildprog", after_build) 