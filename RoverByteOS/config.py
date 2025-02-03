import sys
from utils import hex_to_color
import datetime

"""
This file contains the configuration for the roverbyte.
"""

SAVED_OWNER_NAME = "Chris"
OPENAI_API_KEY = ""
OPENAI_ASSISTANT_ID = "asst_cGUSaSRQGUQmOf1gwpEXxlbJ"

OPENAI_API_KEY = "ENTER OPEN AI API KEY HERE"
OPENAI_ASSISTANT_ID = "ENTER ASSISTANT ID HERE"

INPUT_MODE = 'voice'  # or 'keyboard'
WITH_IMG = True
LANGUAGE = []
VOLUME_DB = 9
TTS_VOICE = 'echo'
VOICE_ACTIONS = ["bark", "bark harder", "pant", "howling"]

# Command line arguments
args = sys.argv[1:]
input_mode = 'voice' if '--keyboard' not in args else 'keyboard'
with_img = False if '--no-img' in args else True

# Other configuration variables
STATUS_STAND = 'stand'
STATUS_SIT = 'sit'
STATUS_LIE = 'lie'
STAND_HEAD_PITCH = 0
SIT_HEAD_PITCH = 20

