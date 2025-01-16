from preset_actions import *
import time

"""
ActionFlow is the main class to control the dog.
It provides a layer of abstraction over the raw actions defined in preset_actions.py

To add a new action, define it in preset_actions.py and add it to the OPERATIONS dictionary.
"""

class ActionFlow:

    #Constants with meaningful names for clearer code
    SIT_HEAD_PITCH = -35 
    STAND_HEAD_PITCH = 0
    STATUS_STAND = 0
    STATUS_SIT = 1
    STATUS_LIE = 2
    HEAD_SPEED = 80
    HEAD_ANGLE = 20
    CHANGE_STATUS_SPEED = 60

    #Dictionary of actions (from preset_actions.py), each with a function and a status
    #The status is used to change the dog's status (sit, stand, lie), for example after a bark action we may want the dog to stand up.
    #The before and after actions are used to chain actions together, for example after a bark action we may want to wag the tail.
    OPERATIONS = {
        "forward": {"function": lambda self: self.dog_obj.do_action('forward', speed=50), "status": STATUS_STAND},
        "backward": {"function": lambda self: self.dog_obj.do_action('backward', speed=50), "status": STATUS_STAND},
        "lie": {"function": lambda self: self.change_status(self.STATUS_LIE), "status": STATUS_LIE},
        "stand": {"function": lambda self: self.change_status(self.STATUS_STAND), "status": STATUS_STAND},
        "sit": {"function": lambda self: self.change_status(self.STATUS_SIT), "status": STATUS_SIT},
        "bark": {"function": lambda self: bark(self.dog_obj), "status": None},
        "bark harder": {"function": lambda self: bark_action(self.dog_obj), "status": None},
        "pant": {"function": lambda self: pant(self.dog_obj), "status": None},
        "howling": {"function": lambda self: howling(self.dog_obj), "status": None},
        "wag_tail": {"function": lambda self: wag_tail(self.dog_obj), "status": None},
        "stretch": {"function": lambda self: stretch(self.dog_obj), "status": None},
        "push up": {"function": lambda self: push_up(self.dog_obj), "status": None},
        "scratch": {"function": lambda self: scratch(self.dog_obj), "status": None},
        "handshake": {"function": lambda self: handshake(self.dog_obj), "status": None},
        "high five": {"function": lambda self: high_five(self.dog_obj), "status": None},
        "lick hand": {"function": lambda self: lick_hand(self.dog_obj), "status": None},
        "shake head": {"function": lambda self: shake_head(self.dog_obj), "status": None},
        "relax neck": {"function": lambda self: relax_neck(self.dog_obj), "status": None},
        "nod": {"function": lambda self: nod(self.dog_obj), "status": None},
        "think": {"function": lambda self: think(self.dog_obj), "status": None},
        "recall": {"function": lambda self: recall(self.dog_obj), "status": None},
        "head down": {"function": lambda self: head_down(self.dog_obj), "status": None},
        "fluster": {"function": lambda self: fluster(self.dog_obj), "status": None},
        "surprise": {"function": lambda self: surprise(self.dog_obj), "status": None},
        "dab": {"function": lambda self: dab(self.dog_obj), "status": None},
        "floss": {"function": lambda self: floss(self.dog_obj), "status": None},
        "woah": {"function": lambda self: woah(self.dog_obj), "status": None},
        "gangnam style": {"function": lambda self: gangnam_style(self.dog_obj), "status": None},
        "bottle flip": {"function": lambda self: bottle_flip(self.dog_obj), "status": None},
        "twerk": {"function": lambda self: twerk_multiple_times(self.dog_obj), "status": None},
        "pray": {"function": lambda self: pray(self.dog_obj), "status": None},
        "butt up": {"function": lambda self: butt_up(self.dog_obj), "status": None},
    }

    def twerk_multiple_times(self):
        for _ in range(5):
            twerk(self.dog_obj)
            time.sleep(0.5)  # Add a small delay between twerks


    def __init__(self, dog_obj):
        self.dog_obj = dog_obj
        self.head_yrp = [0, 0, 0]
        self.head_pitch_init = 0
        self.current_status = self.STATUS_LIE
        self.current_action = None
        self.last_actions = None

    def set_head_pitch_init(self, pitch):
        self.head_pitch_init = pitch
        self.dog_obj.head_move([self.head_yrp], pitch_comp=pitch,
                        immediately=True, speed=self.HEAD_SPEED)
                     
    def change_status(self, status):
        if status == self.STATUS_STAND:
            self.set_head_pitch_init(self.STAND_HEAD_PITCH)
            if self.current_status != self.STATUS_STAND:
                sit_2_stand(self.dog_obj, speed=75) # speed > 70
            else:
               self.dog_obj.do_action('stand', speed=self.CHANGE_STATUS_SPEED) 
        elif status == self.STATUS_SIT:
            self.set_head_pitch_init(self.SIT_HEAD_PITCH)
            self.dog_obj.do_action('sit', speed=self.CHANGE_STATUS_SPEED)
        elif status == self.STATUS_LIE:
            self.set_head_pitch_init(self.STAND_HEAD_PITCH)
            self.dog_obj.do_action('lie', speed=self.CHANGE_STATUS_SPEED)
        
        self.current_status = status
        self.dog_obj.wait_all_done()

    def run(self, action):
        try:
            print(f'\033[90mrun: {action}\033[m')
            if action in self.OPERATIONS:
                operation = self.OPERATIONS[action]
                self.current_action = action
                
                if "status" in operation and operation["status"] != None: #If the action has a status, and the last action was not this action, change the status
                    if self.last_actions != action:
                        self.last_actions = action 
                        self.change_status(operation["status"])

                if "before" in operation and operation["before"] != None: #If the action has a before action, run it
                    before = operation["before"]
                    if before in self.OPERATIONS and self.OPERATIONS[before]["function"] != None:
                        self.OPERATIONS[before]["function"](self)
                        self.dog_obj.wait_all_done()
                    else:
                        before(self)
                        self.dog_obj.wait_all_done()

                if "function" in operation and operation["function"] != None: #If the action has a function, run it
                    operation["function"](self)
                    self.dog_obj.wait_all_done()

                if "after" in operation and operation["after"] != None: #If the action has an after action, run it
                    after = operation["after"]
                    if after in self.OPERATIONS and self.OPERATIONS[after]["function"] != None: 
                        self.OPERATIONS[after]["function"](self)
                        self.dog_obj.wait_all_done()
                    else:
                        after(self)
                        self.dog_obj.wait_all_done()
            else:
                print(f"Unknown action: {action}")
                self.current_action = None
        except Exception as e:
            print(f'action error: {e}')
            self.current_action = None

    def get_current_action(self):
        return self.current_action


def interactive_action_demo(my_dog): #demo all actionss
    action_flow = ActionFlow(my_dog)
    action_flow.change_status(action_flow.STATUS_SIT)
    # action_flow.change_status(action_flow.STATUS_STAND)

    actions = list(action_flow.OPERATIONS.keys())
    for i, key in enumerate(actions):
        print(f'{i} {key}')

    last_key = None

    while True:
        key = input()
        try:
            if key == '':
                if last_key is not None:
                    print(actions[last_key])
                    action_flow.run(actions[last_key])
            else:
                key = int(key)
                last_key = key
                print(actions[key])
                action_flow.run(actions[key])
        except ValueError:
            print('Invalid input')
        except IndexError:
            print('Invalid action number')

if __name__ == '__main__':
    try:
        from pidog import Pidog
        import time
        my_dog = Pidog()
        time.sleep(1)
        my_dog.rgb_strip.set_mode('listen', 'cyan', 1)

        interactive_action_demo(my_dog)
    except KeyboardInterrupt:
        pass
    except Exception as e:
        print(f"\033[31mERROR: {e}\033[m")
    finally:
        my_dog.close()