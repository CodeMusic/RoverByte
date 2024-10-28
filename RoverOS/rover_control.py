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
        self.touch_active = False
        self.motion_active = False
        self.up_flag = False
        self.down_flag = False
        self.setup_threads()

    def setup_threads(self):
        self.speak_thread = threading.Thread(target=self.speak_handler)
        self.speak_thread.daemon = True
        self.action_thread = threading.Thread(target=self.action_handler)
        self.action_thread.daemon = True
        self.touch_thread = threading.Thread(target=self.touch_sensor_handler)
        self.touch_thread.daemon = True
        self.motion_thread = threading.Thread(target=self.motion_handler)
        self.motion_thread.daemon = True

    def motion_handler(self):
        while True:
            try:
                ax = self.dog.accData[0]
                
                # Picked up (acceleration opposite to gravity)
                if ax < -18000:  # More than 1G downward
                    self.dog.body_stop()
                    if not self.up_flag:
                        self.up_flag = True
                    if self.down_flag:
                        # Reset flags when put down
                        self.motion_active = False
                        self.down_flag = False
                        # Return to standing position
                        with self.action_lock:
                            self.actions_to_be_done.clear()
                            self.action_state = 'standby'
                        self.dog.do_action('stand', speed=60)
                        self.set_rgb_mode('breath', 'green')
                        self.dog.wait_legs_done()

                # Lifted up (acceleration same direction as gravity)
                if ax > -13000:  # Less than 1G downward
                    self.dog.body_stop()
                    if self.up_flag and not self.motion_active:
                        self.motion_active = True
                        self.up_flag = False
                        # Execute "flying" celebration
                        with self.action_lock:
                            self.actions_to_be_done.clear()
                            self.action_state = 'standby'
                        # Celebration sequence
                        self.set_rgb_mode('boom', 'red')
                        self.dog.legs.servo_move([45, -45, 90, -80, 90, 90, -90, -90], speed=60)
                        self.dog.do_action('wag_tail', step_count=10, speed=100)
                        self.dog.speak('woohoo', volume=80)
                        self.dog.wait_legs_done()
                    
                    if not self.down_flag:
                        self.down_flag = True

                time.sleep(0.02)  # Quick polling for responsive motion detection
                
            except Exception as e:
                print(f"Motion handler error: {e}")
                time.sleep(0.1)
                
    def touch_sensor_handler(self):
        while True:
            touch_state = self.dog.dual_touch.read()
            if touch_state != 'N':  # Any touch detected
                # Interrupt current actions
                with self.action_lock:
                    self.actions_to_be_done.clear()
                    self.action_state = 'standby'
                
                # Execute touch response sequence
                self.set_rgb_mode('listen', "#8A2BE2")  # Purple color for interaction
                
                # Queue happy response actions
                actions = []
                # Head nod movement
                self.dog.head_move([[0, 0, 0]], immediately=True, speed=90)  # Reset head position
                y, r, p = 0, 0, 30
                head_angles = []
                for i in range(20):
                    r = round(10 * sin(i * 0.314), 2)
                    p = round(20 * sin(i * 0.314) + 10, 2)
                    head_angles.append([y, r, p])
                self.dog.head_move(head_angles, immediately=False, speed=80)
                
                # Tail wag
                self.dog.do_action('wag_tail', step_count=10, speed=80)
                
                # Wait for movements to complete
                self.dog.wait_all_done()
                
                # Reset RGB after interaction
                self.set_rgb_mode('breath', 'pink')
                
                time.sleep(0.2)  # Small delay before accepting next touch
            else:
                # Normal idle state
                self.dog.tail_stop()  # Stop tail movement when not being touched
                time.sleep(0.01)  # Quick polling for responsive touch detection

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
        self.touch_thread.start() 
        self.motion_thread.start()

def close(self):
    """Gracefully close all resources"""
    try:
        # Stop all threads
        self.running = False
        if hasattr(self, 'touch_thread'):
            self.touch_thread.join(timeout=1.0)
        if hasattr(self, 'motion_thread'):
            self.motion_thread.join(timeout=1.0)
        if hasattr(self, 'action_thread'):
            self.action_thread.join(timeout=1.0)
        if hasattr(self, 'speak_thread'):
            self.speak_thread.join(timeout=1.0)
        
        # Close dog resources
        if hasattr(self, 'dog'):
            self.dog.close()
    except Exception as e:
        print(f"Error during RoverControl cleanup: {e}")

