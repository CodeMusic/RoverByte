import os
import sys
import time
import cv2
from vilib import Vilib
from config import INPUT_MODE, WITH_IMG, LANGUAGE, TTS_VOICE, VOLUME_DB
from utils import grey_print, redirect_error_2_null, cancel_redirect_error, get_color_of_the_week, get_color_of_the_week_hex, inverted_color_hex
from ai_interaction import AIInteraction
from rover_control import RoverControl
from math import sin
from voice_commands import VOICE_COMMANDS



"""
This file contains the main function for the roverbyte RoverOS.
"""

# ----------------------------------------------------------------
# work around for ALSA error
# ----------------------------------------------------------------
import ctypes
import ctypes.util

ERROR_HANDLER_FUNC = ctypes.CFUNCTYPE(None, ctypes.c_char_p, ctypes.c_int, ctypes.c_char_p, ctypes.c_int, ctypes.c_char_p)

def py_error_handler(filename, line, function, err, fmt):
    pass

c_error_handler = ERROR_HANDLER_FUNC(py_error_handler)

asound = ctypes.CDLL(ctypes.util.find_library('asound'))
asound.snd_lib_error_set_handler(c_error_handler)
# ----------------------------------------------------------------

def head_nod():
    y = 0
    r = 0
    p = 30
    angs = []
    for i in range(20):
        r = round(10*sin(i*0.314), 2)
        p = round(20*sin(i*0.314) + 10, 2)
        angs.append([y, r, p])
    return angs

def process_voice_command(text, ai, dog):
    command = text.lower()
    commandSafe = command.replace('.', '').replace(',', '').replace('!', '').replace('?', '')
    processed = False
    is_hey_rover = commandSafe.startswith('hey rover') or commandSafe.startswith('hey roverbyte') 

    if is_hey_rover:
        commandSafe = commandSafe.replace('hey rover', '').replace('hey roverbyte', '')
        if commandSafe in VOICE_COMMANDS:
            action = VOICE_COMMANDS[commandSafe]
            dog.action_flow.run(action)
            processed = True
        elif commandSafe.startswith('channel'):
            personToChannel = commandSafe.replace('channel', '')
            tts_file = ai.generate_speech(f"Channeling {personToChannel}...")
            if tts_file:
                dog.dog.speak_block(tts_file)
                grey_print(f"Speech generated: {tts_file}")
            else:
                grey_print("Failed to generate speech (tts_file)")
            processed = True
        else:
            dog.speak("Sorry, I don't know that command.")
            processed = False

    if not processed:
        # If not a direct command, pass to AI
        ai_response = ai.generate_response(text)
        ai_response_clean = ai_response.lower().replace('[', '').replace(']', '').replace('"', '').replace(',', '').replace('.', '').replace('!', '').replace('?', '').strip()
        # Check if AI response contains an action
        for cmd in VOICE_COMMANDS.values():
            if cmd in ai_response_clean:
                dog.action_flow.run(cmd)    
                processed = True
        # If no action in AI response, just speak the response
        #if not processed:
        #    dog.speak(ai_response)

        return processed
    

def main():
    ai = AIInteraction()
    dog = RoverControl()

    dog.dog.rgb_strip.close()
    dog.action_flow.change_status(dog.action_flow.STATUS_SIT)

    _color, _day = get_color_of_the_week()
    _color_hex = get_color_of_the_week_hex()
    _invert_color_hex = inverted_color_hex()
    tts_file = ai.generate_speech(f"Launching RoverOS... RoverByte is waking up!", 'nova')
    tts_file2 = ai.generate_speech(f"I am online, and ready to help on this {_color} {_day}!")  
    if tts_file:
        dog.dog.speak_block(tts_file)
        grey_print(f"Speech generated: {tts_file}")
    else:
        grey_print("Failed to generate speech (tts_file)")

    #stretch
    dog.action_flow.run('stretch')
    #dog.wait_all_done()
    dog.action_flow.run('sit')
    #dog.wait_all_done()
    dog.action_flow.run('bark')
    #dog.wait_all_done()
    dog.action_flow.run('wag_tail')
    #dog.wait_all_done()
    
    if tts_file2:
        dog.dog.speak_block(tts_file2)
        grey_print(f"Speech generated: {tts_file2}")
    else:
        grey_print("Failed to generate speech (tts_file2)")

    if WITH_IMG:
        Vilib.camera_start(vflip=False, hflip=False)
        Vilib.display(local=False, web=True)
        while not Vilib.flask_start:
            time.sleep(0.01)
        time.sleep(0.5)
        print('\n')

    last_action_time = 0
    action_cooldown = 1  # 1 second cooldown between actions

    while True:
        # Handle input
        if INPUT_MODE == 'voice':
            voice_input = handle_voice_input(dog, ai, _color_hex)
            if voice_input:
                action_performed = process_voice_command(voice_input, ai, dog)
                if not action_performed:
                    # If no action was performed, process as before
                    process_input(dog, ai, voice_input)
        elif INPUT_MODE == 'keyboard':
            keyboard_input = handle_keyboard_input(dog, _invert_color_hex)
            if keyboard_input:
                process_input(dog, ai, keyboard_input)
        else:
            raise ValueError("Invalid input mode")

        print() 

def handle_voice_input(dog, ai, color_hex):
    grey_print("listening ...")
    dog.set_action_state('standby')
    dog.set_rgb_mode('listen', color_hex)

    _stderr_back = redirect_error_2_null()
    audio = ai.listen_for_audio()
    cancel_redirect_error(_stderr_back)

    dog.set_rgb_mode('boom', '#ffff00')

    return ai.speech_to_text(audio, LANGUAGE)

def handle_keyboard_input(dog, invert_color_hex):
    dog.set_action_state('standby')
    dog.set_rgb_mode('listen', invert_color_hex)
    return input(f'\033[1;30m{"input: "}\033[0m').encode(sys.stdin.encoding).decode('utf-8')

def process_input(dog, ai, input_text):
    img_path = './img_input.jpg'
    cv2.imwrite(img_path, Vilib.img)

    dog.set_action_state('think')

    st = time.time()
    response = ai.dialogue_with_img(input_text, img_path)
    grey_print(f'chat takes: {time.time() - st:.3f} s')

    response_list = ai.parse_response(response)
    grey_print(f'response_list: {response_list}')

    for item in response_list:
        if item['key'] == 'action':
            dog.action_flow.run(item['value'])
            time.sleep(0.5)
        elif item['key'] == 'content':
            tts_file = ai.generate_speech(item['value'])
            if tts_file:
                dog.dog.speak_block(tts_file)
                dog.set_rgb_mode('speak', '#0000ff')
            else:
                dog.set_rgb_mode('breath', get_color_of_the_week_hex())

    dog.action_flow.run('sit')  # return to sit position

def cleanup():
    print("\nShutting down RoverOS gracefully...")
    try:
        if WITH_IMG:
            print("Closing camera...")
            Vilib.camera_close()
        
        print("Stopping all actions...")
        dog = RoverControl()
        # Stop any ongoing actions
        dog.dog.body_stop()
        dog.dog.head_stop()
        dog.dog.tail_stop()
        
        # Return to neutral position
        print("Returning to neutral position...")
        dog.action_flow.run('lie')
        dog.dog.wait_all_done()
        
        # Turn off RGB
        print("Turning off RGB strip...")
        try:
            dog.dog.rgb_strip.close()
        except:
            pass
        
        # Close audio
        print("Closing audio systems...")
        try:
            dog.dog.sound_direction.close()
            dog.dog.sound_effect.close()
        except:
            pass
        
        # Final cleanup
        print("Closing RoverControl...")
        
    except Exception as e:
        print(f"\033[31mError during cleanup: {e}\033[m")
    
    print("Shutdown complete.")

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nReceived shutdown signal (Ctrl+C)")
    except Exception as e:
        print(f"\033[31mERROR: {e}\033[m")
    finally:
        cleanup()