import datetime
import subprocess
import sys
import shutil
import time

"""
This file contains utility functions for the roverbyte.
"""

def grey_print(text):
    print(f'\033[1;30m{text}\033[0m')

def hex_to_color(hex_string):
    hex_string = hex_string.lstrip('#')
    rgb = tuple(int(hex_string[i:i+2], 16) for i in (0, 2, 4))
    return tuple(x / 255.0 for x in rgb)

def get_color_of_the_week():
    colors = [
        ("Red", "Sunday"),
        ("Orange", "Monday"),
        ("Yellow", "Tuesday"),
        ("Green", "Wednesday"),
        ("Blue", "Thursday"),
        ("Indigo", "Friday"),
        ("Violet", "Saturday")
    ]
    # Use (datetime.datetime.now().weekday() + 1) % 7 to shift Sunday to 0
    today = (datetime.datetime.now().weekday() + 1) % 7
    return colors[today]

def get_color_of_the_week_hex():
    color_map = {
        "Red": "#FF0000",
        "Orange": "#FFA500",
        "Yellow": "#FFFF00",
        "Green": "#00FF00",
        "Blue": "#0000FF",
        "Indigo": "#4B0082",
        "Violet": "#8A2BE2"
    }
    color, _ = get_color_of_the_week()
    return color_map[color]

def inverted_color_hex():
    original_hex = get_color_of_the_week_hex()
    original_hex = original_hex.lstrip('#')  # Remove the '#' character
    rgb = tuple(int(original_hex[i:i+2], 16) for i in (0, 2, 4)) # Convert hex to RGB
    inverted_rgb = tuple(255 - value for value in rgb) # Invert the RGB values
    inverted_hex = '#{:02x}{:02x}{:02x}'.format(*inverted_rgb) # Convert inverted RGB back to hex
    return inverted_hex

def sox_volume(input_file, output_file, volume_db):
    cmd = f'sox {input_file} {output_file} vol {volume_db}dB'
    subprocess.call(cmd, shell=True)
    return True


def redirect_error_2_null():
    stderr_backup = sys.stderr
    sys.stderr = open('/dev/null', 'w')
    return stderr_backup

def cancel_redirect_error(stderr_backup):
    sys.stderr.close()
    sys.stderr = stderr_backup

    # utils
# =================================================================
def chat_print(label, message):
    width = shutil.get_terminal_size().columns
    msg_len = len(message)
    line_len = width - 27

    # --- normal print ---
    print(f'{time.time():.3f} {label:>6} >>> {message}')
    return

    # --- table mode ---
    if width < 38 or msg_len <= line_len:
        print(f'{time.time():.3f} {label:>6} >>> {message}')
    else:
        texts = []

        # words = message.split()
        # print(words)
        # current_line = ""
        # for word in words:
        #     if len(current_line) + len(word) + 1 <= line_len:
        #         current_line += word + " "
        #     else:
        #         texts.append(current_line)
        #         current_line = ""

        # if current_line:
        #     texts.append(current_line)

        for i in range(0, len(message), line_len):
            texts.append(message[i:i+line_len])

        for i, text in enumerate(texts):
            if i == 0:
                print(f'{time.time():.3f} {label:>6} >>> {text}')
            else:
                print(f'{"":>26} {text}')