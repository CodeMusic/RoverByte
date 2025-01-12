import threading
import time

EVENT_THREAD_DEFAULT_STACK_SIZE = 10240

EVENT_STATUS_FATAL_ERROR = -1
EVENT_STATUS_NOT_START = 0
EVENT_STATUS_READY = 1
EVENT_STATUS_RUNNING = 2

current_events_info = []

##### event trigger  
TRIGGER_NEVER = 0                  # the event of this type will be never trriggered
TRIGGER_ALWAYS_WITH_NO_PARAMETER = 1   # the event of this type will be always trriggered
TRIGGER_ONCE_BY_VALUE_TRUE = 2         # trigger once when the condition comes true from false
TRIGGER_CONTINUOUS_BY_VALUE_TRUE = 3   # trigger multiple times when the condition being true
TRIGGER_ONCE_BY_VALUE_LARGER = 4       # trigger once when the threshold is larger than the parameter in
TRIGGER_CONTINUOUS_BY_VALUE_LARGER = 5 # trigger multiple times when the threshold is larger than the parameter in
TRIGGER_ONCE_BY_VALUE_SMALLER = 6      # trigger once when the threshold is smaller than the parameter in
TRIGGER_CONTINUOUS_BY_VALUE_SMALLER = 7# trigger multiple times when the threshold is smaller than the parameter in
TRIGGER_BY_STRING_MATCHING = 8         # trigger once when the target string is matched with the parameter in
TRIGGER_ONCE_BY_VALUE_TRUE_WITH_OPTION = 9  # a special type, we do not use it now
TRIGGER_ALWAYS_WITH_NOT_NONE = 10   # 

##### event type
EVE_SYSTEM_LAUNCH = 1
EVE_TIME_OVER = 2
EVE_MESSAGE = 3
EVE_CLOUD_MESSAGE = 4
EVE_MESH_MESSAGE = 5
EVE_UPLOAD_MODE_MESSAGE = 6
EVE_SYSTEM_RUN_APP = 7

EVENT_BUTTON = 11
EVENT_BUTTON1 = 12
EVENT_BUTTON2 = 13

EVENT_JOYSTICK_UP = 14
EVENT_JOYSTICK_DOWN = 15
EVENT_JOYSTICK_RIGHT = 16
EVENT_JOYSTICK_LEFT = 17
EVENT_JOYSTICK_CENTER = 18

EVENT_BUTTON_ANY = 19
EVENT_JOYSTICK_ANY = 20
EVENT_JOYSTICK_BUTTON_ANY = 21

EVENT_SHAKED = 22
EVENT_TILT_LEFT = 23
EVENT_TILT_RIGHT = 24
EVENT_TILT_FORWARD = 25
EVENT_TILT_BACKWARD = 26
EVENT_SCREEN_UP = 27
EVENT_SCREEN_DOWN = 28
EVENT_UPRIGHT = 29
EVENT_ROTATE_CLOCKWISE = 30
EVENT_ROTATE_ANTICLOCKWISE = 31
EVENT_FREE_FALL = 32
EVENT_BRANDISH_UP = 33
EVENT_BRANDISH_DOWN = 34
EVENT_BRANDISH_LEFT = 35
EVENT_BRANDISH_RIGHT = 36
    
EVENT_MICROPHONE = 37

EVENT_LIGHT_SENSOR = 38


def eve_trigger_check(eve_type, para_in):
    if eve_type.trigger_type == TRIGGER_ALWAYS_WITH_NO_PARAMETER:
        return True
    elif eve_type.trigger_type == TRIGGER_ONCE_BY_VALUE_TRUE:
        return para_in
    elif eve_type.trigger_type == TRIGGER_ONCE_BY_VALUE_LARGER:
        return para_in > eve_type.user_para
    elif eve_type.trigger_type == TRIGGER_ONCE_BY_VALUE_SMALLER:
        return para_in < eve_type.user_para
    elif eve_type.trigger_type == TRIGGER_ONCE_BY_VALUE_TRUE_WITH_OPTION:
        return para_in == eve_type.user_para
    elif eve_type.trigger_type == TRIGGER_ALWAYS_WITH_NOT_NONE:
        return para_in != None 

class event_class_c(object):
    def __init__(self, eve_id, event_type, trigger_type, user_cb, user_para = 0, stack_size = EVENT_THREAD_DEFAULT_STACK_SIZE):
        self.event_type = event_type
        self.trigger_type = trigger_type
        self.user_cb = user_cb
        self.user_para = user_para
        self.stack_size = stack_size
        self.event_status = EVENT_STATUS_NOT_START
        self.eve_id = eve_id
        self.sys_lock = threading.Lock()
        self.sys_lock.acquire()

        self.handle = threading.Thread(target = self.__event_cb_task, args = ()) 
        self.handle.start()

        self.event_triggered_flag = False

    def __event_cb_task(self):
        while True:
            self.event_status = EVENT_STATUS_READY
            # blocking when event not be triggered
            if self._wait_trigger() == True:
                # Call user callback function
                if self.user_cb:
                    self.event_status = EVENT_STATUS_RUNNING
                    self.user_cb()
            else:
                continue

    def _wait_trigger(self):
        self.sys_lock.acquire()
        return True

    def trigger(self, para):
        if self.trigger_type >= TRIGGER_ONCE_BY_VALUE_TRUE and  self.trigger_type <= TRIGGER_CONTINUOUS_BY_VALUE_SMALLER:
            if (self.event_triggered_flag == False) and  eve_trigger_check(self, para):
                self.sys_lock.release()
                self.event_triggered_flag = True
            elif not eve_trigger_check(self, para):
                self.event_triggered_flag = False
        else:
            if eve_trigger_check(self, para):
                self.sys_lock.release()

    def get_type(self):
        return self.event_type
            
######################################################################################
def event_response_cb(pack):
    global current_events_info
    for item in current_events_info:
        if item.eve_id == pack.subscribe_key:
            item.trigger(pack.subscribe_value)

def event_trigger(eve_type, parameter = None):
    global current_events_info
    for item in current_events_info:
        if item.get_type() == eve_type:
            item.trigger(parameter)

def event_register(eve_id, event_type, trigger_type, user_cb, user_para = 0, stack_size = EVENT_THREAD_DEFAULT_STACK_SIZE):
    global current_events_info
    if eve_id > 0:
        current_events_info.append(event_class_c(eve_id, event_type, trigger_type, user_cb, user_para, stack_size))


