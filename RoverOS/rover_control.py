from pidog import Pidog
from action_flow import *
import threading
import time

"""
This file contains the RoverControl abstraction for the roverbyte pidog libraries.
It also establishes handlers, and threads to help with the control and flow of roverbyte.
"""

class RoverControl:
    def __init__(self):
        self.dog = Pidog()
        self.action_flow = ActionFlow(self.dog)
        self.action_state = 'standby'
        self.actions_to_be_done = []
        self.speech_loaded = False
        self.tts_file = None
        self.action_lock = threading.Lock()
        self.speech_lock = threading.Lock()
        self.setup_threads()

    def setup_threads(self):
        self.speak_thread = threading.Thread(target=self.speak_handler)
        self.speak_thread.daemon = True
        self.action_thread = threading.Thread(target=self.action_handler)
        self.action_thread.daemon = True

    def speak_handler(self):
        while True:
            with self.speech_lock:
                if self.speech_loaded:
                    self.dog.speak_block(self.tts_file)
                    self.speech_loaded = False
            time.sleep(0.05)

    def action_handler(self):
        while True:
            with self.action_lock:
                if self.action_state == 'actions' and self.actions_to_be_done:
                    action = self.actions_to_be_done.pop(0)
                    self.execute_action(action)
                    if not self.actions_to_be_done:
                        self.action_state = 'standby'
            time.sleep(0.05)

    def execute_action(self, action):
        action = action.lower()
        if action in self.action_flow.OPERATIONS:
            self.action_flow.change_status(action)
        else:
            print(f"Unknown action: {action}")

    def set_action_state(self, state):
        with self.action_lock:
            self.action_state = state

    def set_rgb_mode(self, mode, color):
        try:
            self.dog.rgb_strip.set_mode(mode, color, 1)
        except Exception as e:
            print(f'set rgb strip error: {e}')

    def speak(self, tts_file):
        self.tts_file = tts_file
        with self.speech_lock:
            self.speech_loaded = True

    def execute_actions(self, actions):
        with self.action_lock:
            self.actions_to_be_done.extend(actions)
            self.action_state = 'actions'

    def wait_for_actions_to_finish(self):
        while True:
            with self.action_lock:
                if self.action_state != 'actions':
                    break
            time.sleep(0.01)

    def wait_for_speech_to_finish(self):
        while True:
            with self.speech_lock:
                if not self.speech_loaded:
                    break
            time.sleep(0.01)

    def start_threads(self):
        self.speak_thread.start()
        self.action_thread.start()

    def close(self):
        self.dog.close()

    def touch_sensor_handler(self):  #WIP
        while True:
            if self.dog.touch_sensor.is_touched():
                print("Touch sensor activated!")
                # Add any specific actions you want to perform when the touch sensor is activated
            time.sleep(0.1)  # Check every 100ms