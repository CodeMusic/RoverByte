from makeblock.modules.cyberpi import event_manager 

module_obj = None
def set_mdoule_obj(module):
    global module_obj
    module_obj = module

def get_eve_id(tag, tag_para):
    global module_obj
    if module_obj == None:
        return -1

    return module_obj.register_event(tag, tag_para, event_manager.event_response_cb)

controller_id_table = {"up":0, "down":1, "right":2, "left":3, "center":4, "middle":4, "a": 1, "b": 0}
# event type definition

def start(callback):
    event_manager.event_register(0xffff, event_manager.EVE_SYSTEM_LAUNCH, event_manager.TRIGGER_ALWAYS_WITH_NO_PARAMETER, callback, None)
    event_manager.event_trigger(event_manager.EVE_SYSTEM_LAUNCH)

def is_press(id):
    def decorator(callback):
        id_str = str(id)
        if id_str == "center":
            id_str = 'middle'

        if id_str == "a" or id_str == "b":
            eve_id = get_eve_id("0f4008600f10c30d9500bf0896cb3945", id_str)
            event_manager.event_register(eve_id, event_manager.EVENT_BUTTON + controller_id_table[id_str], event_manager.TRIGGER_ONCE_BY_VALUE_TRUE, callback, None)
        elif id_str in controller_id_table:
            eve_id = get_eve_id("0f4008600f10c30d9500bf0896cb3945", id_str)
            event_manager.event_register(eve_id, event_manager.EVENT_JOYSTICK_UP + controller_id_table[id_str], event_manager.TRIGGER_ONCE_BY_VALUE_TRUE, callback, None)
        elif id_str == "any_direction":
            eve_id = get_eve_id("0f4008600f10c30d9500bf0896cb3945", id_str)
            event_manager.event_register(eve_id, event_manager.EVENT_JOYSTICK_ANY, event_manager.TRIGGER_ONCE_BY_VALUE_TRUE, callback, None)
        elif id_str == "any_button":
            eve_id = get_eve_id("0f4008600f10c30d9500bf0896cb3945", id_str)
            event_manager.event_register(eve_id, event_manager.EVENT_BUTTON_ANY, event_manager.TRIGGER_ONCE_BY_VALUE_TRUE, callback, None)
        elif id_str == "any":
            eve_id = get_eve_id("0f4008600f10c30d9500bf0896cb3945", id_str)
            event_manager.event_register(eve_id, event_manager.EVENT_JOYSTICK_BUTTON_ANY, event_manager.TRIGGER_ONCE_BY_VALUE_TRUE, callback, None)
                                      
    return decorator    

def wifi_broadcast(message):
    def decorator(callback):
        mstr_str = str(message)
        eve_id = get_eve_id("7fa7ab3b41e128adefb209e63132a738", mstr_str)
        event_manager.event_register(eve_id, event_manager.EVE_MESH_MESSAGE, event_manager.TRIGGER_ALWAYS_WITH_NOT_NONE, callback, mstr_str)
    return decorator

def cloud_broadcast(message):
    def decorator(callback):
        mstr_str = str(message)
        eve_id = get_eve_id("db996e364162c972990c3a7497e74d38", mstr_str)
        event_manager.event_register(eve_id, event_manager.EVE_CLOUD_MESSAGE, event_manager.TRIGGER_ALWAYS_WITH_NOT_NONE, callback, mstr_str)
    return decorator

def mesh_broadcast(message):
    def decorator(callback):
        mstr_str = str(message)
        eve_id = get_eve_id("7fa7ab3b41e128adefb209e63132a738", mstr_str)
        event_manager.event_register(eve_id, event_manager.EVE_MESH_MESSAGE, event_manager.TRIGGER_ALWAYS_WITH_NOT_NONE, callback, mstr_str)
    return decorator

def greater_than(threshold, mode):
    def decorator(callback):
        if not isinstance(threshold, (int, float)):
            return
        threshold_data = threshold
        if threshold_data < 0:
            threshold_data = 0
        type_str = mode
        if type_str == "microphone":
            eve_id = get_eve_id("2469bdefeb4529102292af589e8a2efc", ())
            event_manager.event_register(eve_id, event_manager.EVENT_MICROPHONE, event_manager.TRIGGER_ONCE_BY_VALUE_LARGER, callback, threshold_data)
        elif type_str == "timer":
            eve_id = get_eve_id("5535075a55d873b7450bad031ba3ba72", ())
            event_manager.event_register(eve_id, event_manager.EVE_TIME_OVER, event_manager.TRIGGER_ONCE_BY_VALUE_LARGER, callback, threshold_data)
        elif type_str == "light_sensor":
            eve_id = get_eve_id("04eeffd3a88cf7eee2bf0cb141518cfd", ())
            event_manager.event_register(eve_id, event_manager.EVENT_LIGHT_SENSOR, event_manager.TRIGGER_ONCE_BY_VALUE_LARGER, callback, threshold_data)
        elif type_str == "shake_val":
            eve_id = get_eve_id("8a5db13c5c596e0407aa4abbe4f1db48", ())
            event_manager.event_register(eve_id, event_manager.EVENT_SHAKED, event_manager.TRIGGER_ONCE_BY_VALUE_LARGER, callback, threshold_data)
    return decorator

def smaller_than(threshold, mode):
    def decorator(callback):
        if not isinstance(threshold, (int, float)):
            return
        threshold_data = threshold
        if threshold_data < 0:
            threshold_data = 0
        type_str = mode
        if type_str == "microphone":
            eve_id = get_eve_id("2469bdefeb4529102292af589e8a2efc", ())
            event_manager.event_register(eve_id, event_manager.EVENT_MICROPHONE, event_manager.TRIGGER_ONCE_BY_VALUE_SMALLER, callback, threshold_data)
        elif type_str == "timer":
            eve_id = get_eve_id("5535075a55d873b7450bad031ba3ba72", ())
            event_manager.event_register(eve_id, event_manager.EVE_TIME_OVER, event_manager.TRIGGER_ONCE_BY_VALUE_SMALLER, callback, threshold_data)            
        elif type_str == "light_sensor":
            eve_id = get_eve_id("04eeffd3a88cf7eee2bf0cb141518cfd", ())
            event_manager.event_register(eve_id, event_manager.EVENT_LIGHT_SENSOR, event_manager.TRIGGER_ONCE_BY_VALUE_SMALLER, callback, threshold_data)
        elif type_str == "shake_val":
            eve_id = get_eve_id("8a5db13c5c596e0407aa4abbe4f1db48", ())
            event_manager.event_register(eve_id, event_manager.EVENT_SHAKED, event_manager.TRIGGER_ONCE_BY_VALUE_SMALLER, callback, threshold_data)        
    return decorator

def is_shake(callback):        
    eve_id = get_eve_id("c33fcd8e57ae1e3b4c513f6f3386b4b6", ())
    event_manager.event_register(eve_id, event_manager.EVENT_SHAKED, event_manager.TRIGGER_ONCE_BY_VALUE_TRUE, callback, None)

def is_tiltleft(callback):
    eve_id = get_eve_id("a4cf641a79cf925a87725098646118f9", ())
    event_manager.event_register(eve_id, event_manager.EVENT_TILT_LEFT, event_manager.TRIGGER_ONCE_BY_VALUE_TRUE, callback, None)

def is_tiltright(callback):
    eve_id = get_eve_id("fec52f5ad18094161cec1b417c7e10f5", ())
    event_manager.event_register(eve_id, event_manager.EVENT_TILT_RIGHT, event_manager.TRIGGER_ONCE_BY_VALUE_TRUE, callback, None)

def is_tiltforward(callback):
    eve_id = get_eve_id("aeefd935140b9ce16ede5da7b7ada9fa", ())
    event_manager.event_register(eve_id, event_manager.EVENT_TILT_FORWARD, event_manager.TRIGGER_ONCE_BY_VALUE_TRUE, callback, None)

def is_tiltback(callback):
    eve_id = get_eve_id("4cf92569435ea2828f6e3b6304907430", ())
    event_manager.event_register(eve_id, event_manager.EVENT_TILT_BACKWARD, event_manager.TRIGGER_ONCE_BY_VALUE_TRUE, callback, None)

def is_faceup(callback):
    eve_id = get_eve_id("43c455ca4f76e8a737618fbef054bdbe", ())
    event_manager.event_register(eve_id, event_manager.EVENT_SCREEN_UP, event_manager.TRIGGER_ONCE_BY_VALUE_TRUE, callback, None)

def is_facedown(callback):
    eve_id = get_eve_id("c6830cf56b1a4f9aaee9ab53e89a923d", ())
    event_manager.event_register(eve_id, event_manager.EVENT_SCREEN_DOWN, event_manager.TRIGGER_ONCE_BY_VALUE_TRUE, callback, None)

def is_stand(callback):
    eve_id = get_eve_id("c1fb2bd86ac930a853470c2e99c81ea2", ())
    event_manager.event_register(eve_id, event_manager.EVENT_UPRIGHT, event_manager.TRIGGER_ONCE_BY_VALUE_TRUE, callback, None)


def is_clockwise(callback):
    eve_id = get_eve_id("fe32241d6feac2a268f7eafa488761eb", ())
    event_manager.event_register(eve_id, event_manager.EVENT_ROTATE_CLOCKWISE, event_manager.TRIGGER_ONCE_BY_VALUE_TRUE, callback, None)

def is_anticlockwise(callback):
    eve_id = get_eve_id("196145516d82a638b39657e5b657df7c", ())
    event_manager.event_register(eve_id, event_manager.EVENT_ROTATE_ANTICLOCKWISE, event_manager.TRIGGER_ONCE_BY_VALUE_TRUE, callback, None)

def is_freefall(callback):
    eve_id = get_eve_id("dfabbe0e342dffec53fedfff55bff3e1", ())
    event_manager.event_register(eve_id, event_manager.EVENT_FREE_FALL, event_manager.TRIGGER_ONCE_BY_VALUE_TRUE, callback, None)


def is_waveup(callback):
    eve_id = get_eve_id("e880c1ca448156bf3f371490414e4ee8", ())
    event_manager.event_register(eve_id, event_manager.EVENT_BRANDISH_UP, event_manager.TRIGGER_ONCE_BY_VALUE_TRUE, callback, None)

def is_wavedown(callback):
    eve_id = get_eve_id("aefc483bd0bcd4561ee6d6d2abface7c", ())
    event_manager.event_register(eve_id, event_manager.EVENT_BRANDISH_DOWN, event_manager.TRIGGER_ONCE_BY_VALUE_TRUE, callback, None)

def is_waveleft(callback):
    eve_id = get_eve_id("2dd513e5fbaf94cfe7cafdc64ebbeb74", ())
    event_manager.event_register(eve_id, event_manager.EVENT_BRANDISH_LEFT, event_manager.TRIGGER_ONCE_BY_VALUE_TRUE, callback, None)

def is_waveright(callback):
    eve_id = get_eve_id("231b8c48c8b30f532899415153ce868e", ())
    event_manager.event_register(eve_id, event_manager.EVENT_BRANDISH_RIGHT, event_manager.TRIGGER_ONCE_BY_VALUE_TRUE, callback, None)
