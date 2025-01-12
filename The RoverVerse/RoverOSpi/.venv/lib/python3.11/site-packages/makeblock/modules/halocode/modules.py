# -*- coding: utf-8 -*
import json
import makeblock.utils
from makeblock.protocols.PackData import HalocodePackData

class _BaseModule:
    _pack = None
    _is_received = False
    _last_time = 0
    _callback = None
    def __init__(self,board,index=0):
        self.setup(board,index)

    def setup(self,board,index):
        board._autoconnect()
        self._board = board
        self._index = index
        self._pack = HalocodePackData()
        self._init_module()
    
    def _init_module(self):
        pass

    def request(self,pack):
        self._board.remove_response(pack)
        self._board.request(pack)

    def call(self,pack):
        self._board.call(pack)
    
    def send_script(self,mode,script):
        self._pack.mode = mode
        self._pack.script = script
        if mode==HalocodePackData.TYPE_RUN_WITHOUT_RESPONSE:
            self.call(self._pack)
        if mode==HalocodePackData.TYPE_RUN_WITH_RESPONSE:
            self.request(self._pack)

    def subscribe(self,pack):
        self._board.subscribe(pack)

    def unsubscribe(self,pack):
        self._board.unsubscribe(pack)

class Led(_BaseModule):
    def _init_module(self):
        self._pack.type = HalocodePackData.TYPE_SCRIPT

    def set_pixels(self,pixels):
        # self._pack.data = [0x1]
        # self._pack.data.extend(makeblock.utils.ushort2bits(red))
        # self._pack.data.extend(makeblock.utils.ushort2bits(green))
        # self._pack.data.extend(makeblock.utils.ushort2bits(blue))
        self.call(self._pack)
    
    def start(self):
        script = "led.__init__()"
        self.send_script(HalocodePackData.TYPE_RUN_WITHOUT_RESPONSE,script)

    def set_color(self,index,red,green,blue):
        script = "led.show_single({0},{1},{2},{3})".format(index,red,green,blue)
        self.send_script(HalocodePackData.TYPE_RUN_WITHOUT_RESPONSE,script)

    def set_colors(self,red,green,blue):
        script = "led.show_all({0},{1},{2})".format(red,green,blue)
        self.send_script(HalocodePackData.TYPE_RUN_WITHOUT_RESPONSE,script)

    def set_full_colors(self,colors):
        script = "led.show_full_color({0})".format(colors)
        self.send_script(HalocodePackData.TYPE_RUN_WITHOUT_RESPONSE,script)

class Button(_BaseModule):
    def _init_module(self):
        self._pack.on_response = self.__on_parse
        self._is_pressed = False
        self.subscribe_pressed()

    def __on_parse(self, pack):
        try:
            ret = eval("".join([ chr(i) for i in pack.data[3:len(pack.data)]]))
            if not ret['ret'] is None:
                self._is_pressed = ret['ret']
                if not self._callback==None:
                    self._callback(ret["ret"])
        except:
            print("error")
        else:
            pass

    @property
    def is_pressed(self):
        return self._is_pressed

    def on_subscribe_response(self,pack):
        self._is_pressed = pack.subscribe_value

    def request_pressed(self,callback):
        self._callback = callback
        self._pack.type = HalocodePackData.TYPE_SCRIPT
        script = "button.is_pressed()"
        self.send_script(HalocodePackData.TYPE_RUN_WITH_RESPONSE,script)

    def subscribe_pressed(self):
        pack = HalocodePackData()
        pack.type = HalocodePackData.TYPE_SCRIPT
        pack.script = "subscribe.add_item({0}, button.is_pressed, ())"
        pack.mode = HalocodePackData.TYPE_RUN_WITH_RESPONSE
        pack.on_response = self.on_subscribe_response
        self.subscribe(pack)

class Pin(_BaseModule):
    def _init_module(self):
        self._pack.on_response = self.__on_parse
        self._is_touched = False
        self.subscribe_touched()

    def __on_parse(self, pack):
        try:
            ret = eval("".join([ chr(i) for i in pack.data[3:len(pack.data)]]))
            if not ret['ret'] is None:
                self._is_touched = ret['ret']
                if not self._callback==None:
                    self._callback(ret["ret"])
        except:
            print("error")
        else:
            pass

    @property
    def is_touched(self):
        return self._is_touched

    def on_subscribe_response(self,pack):
        self._is_touched = pack.subscribe_value

    def read_digital(self,callback):
        self.unsubscribe_touched()
        self._callback = callback
        script = "pin{0}.read_digital()".format(self._index)
        self.send_script(HalocodePackData.TYPE_RUN_WITH_RESPONSE,script)

    def read_analog(self,callback):
        self.unsubscribe_touched()
        self._callback = callback
        script = "pin{0}.read_analog()".format(self._index)
        self.send_script(HalocodePackData.TYPE_RUN_WITH_RESPONSE,script)

    def subscribe_touched(self):
        self._pack.type = HalocodePackData.TYPE_SCRIPT
        self._pack.script = "subscribe.add_item({0}, pin{1}.is_touched, ())".format('{0}',self._index)
        self._pack.mode = HalocodePackData.TYPE_RUN_WITH_RESPONSE
        self._pack.on_response = self.on_subscribe_response
        self.subscribe(self._pack)

    def unsubscribe_touched(self):
        self._pack.on_response = self.__on_parse
        pack = HalocodePackData()
        pack.type = HalocodePackData.TYPE_SCRIPT
        pack.script = "subscribe.del_item({0})".format(self._pack.subscribe_key)
        self.unsubscribe(pack)

    def write_digital(self,level):
        self.unsubscribe_touched()
        script = "pin{0}.write_digital({1})".format(self._index,level)
        self.send_script(HalocodePackData.TYPE_RUN_WITHOUT_RESPONSE,script)

    def write_analog(self,pwm):
        self.unsubscribe_touched()
        script = "pin{0}.write_analog({1})".format(self._index,pwm)
        self.send_script(HalocodePackData.TYPE_RUN_WITHOUT_RESPONSE,script)

    def servo_write(self,angle):
        self.unsubscribe_touched()
        script = "pin{0}.servo_write({1})".format(self._index,angle)
        self.send_script(HalocodePackData.TYPE_RUN_WITHOUT_RESPONSE,script)

class Motion(_BaseModule):
    def _init_module(self):
        self._pack.on_response = self.__on_parse
        self._motion = {"roll":0.0,"pitch":0.0,"yaw":0.0,"accel_x":0.0,"accel_y":0.0,"accel_z":0.0,"gyro_x":0.0,"gyro_y":0.0,"gyro_z":0.0,"rotation_x":0.0,"rotation_y":0.0,"rotation_z":0.0,"is_roll_left":False,"is_roll_right":False,"is_pitch_up":False,"is_pitch_down":False,"is_shaking":False,"shake_strength":0.0}
        self.subscribe_motion()

    def __on_parse(self, pack):
        try:
            ret = eval("".join([ chr(i) for i in pack.data[3:len(pack.data)]]))
            if not ret['ret'] is None:
                self._is_touched = ret['ret']
                if not self._callback==None:
                    self._callback(ret["ret"])
        except:
            print("error")
        else:
            pass

    @property
    def roll(self):
        return self._motion['roll']

    @property
    def pitch(self):
        return self._motion['pitch']
         
    @property
    def yaw(self):
        return self._motion['yaw']

    @property
    def is_shaking(self):
        return self._motion['is_shaking']
        
    @property
    def shake_strength(self):
        return self._motion['shake_strength']

    def on_subscribe_response(self,pack):
        self._motion[pack.value_name] = pack.subscribe_value

    def subscribe_motion(self):
        pack = HalocodePackData()
        pack.value_name = "roll"
        pack.type = HalocodePackData.TYPE_SCRIPT
        pack.mode = HalocodePackData.TYPE_RUN_WITH_RESPONSE
        pack.on_response = self.on_subscribe_response
        pack.script = "subscribe.add_item({0}, motion_sensor.get_roll, ())"
        self.subscribe(pack)
        pack = HalocodePackData()
        pack.value_name = "pitch"
        pack.type = HalocodePackData.TYPE_SCRIPT
        pack.mode = HalocodePackData.TYPE_RUN_WITH_RESPONSE
        pack.on_response = self.on_subscribe_response
        pack.script = "subscribe.add_item({0}, motion_sensor.get_pitch, ())"
        self.subscribe(pack)
        pack = HalocodePackData()
        pack.value_name = "yaw"
        pack.type = HalocodePackData.TYPE_SCRIPT
        pack.mode = HalocodePackData.TYPE_RUN_WITH_RESPONSE
        pack.on_response = self.on_subscribe_response
        pack.script = "subscribe.add_item({0}, motion_sensor.get_yaw, ())"
        self.subscribe(pack)
        pack = HalocodePackData()
        pack.value_name = "is_shaking"
        pack.type = HalocodePackData.TYPE_SCRIPT
        pack.mode = HalocodePackData.TYPE_RUN_WITH_RESPONSE
        pack.on_response = self.on_subscribe_response
        pack.script = "subscribe.add_item({0}, motion_sensor.is_shaked, ())"
        self.subscribe(pack)
        pack = HalocodePackData()
        pack.value_name = "shake_strength"
        pack.type = HalocodePackData.TYPE_SCRIPT
        pack.mode = HalocodePackData.TYPE_RUN_WITH_RESPONSE
        pack.on_response = self.on_subscribe_response
        pack.script = "subscribe.add_item({0}, motion_sensor.get_shake_strength, ())"
        self.subscribe(pack)
        pack = HalocodePackData()
        pack.value_name = "is_tilted_left"
        pack.type = HalocodePackData.TYPE_SCRIPT
        pack.mode = HalocodePackData.TYPE_RUN_WITH_RESPONSE
        pack.on_response = self.on_subscribe_response
        pack.script = "subscribe.add_item({0}, motion_sensor.is_tilted_left, ())"
        self.subscribe(pack)
        pack = HalocodePackData()
        pack.value_name = "is_tilted_right"
        pack.type = HalocodePackData.TYPE_SCRIPT
        pack.mode = HalocodePackData.TYPE_RUN_WITH_RESPONSE
        pack.on_response = self.on_subscribe_response
        pack.script = "subscribe.add_item({0}, motion_sensor.is_tilted_right, ())"
        self.subscribe(pack)
        pack = HalocodePackData()
        pack.value_name = "is_arrow_up"
        pack.type = HalocodePackData.TYPE_SCRIPT
        pack.mode = HalocodePackData.TYPE_RUN_WITH_RESPONSE
        pack.on_response = self.on_subscribe_response
        pack.script = "subscribe.add_item({0}, motion_sensor.is_arrow_up, ())"
        self.subscribe(pack)
        pack = HalocodePackData()
        pack.value_name = "is_arrow_down"
        pack.type = HalocodePackData.TYPE_SCRIPT
        pack.mode = HalocodePackData.TYPE_RUN_WITH_RESPONSE
        pack.on_response = self.on_subscribe_response
        pack.script = "subscribe.add_item({0}, motion_sensor.is_arrow_down, ())"
        self.subscribe(pack)

class Microphone(_BaseModule):
    def _init_module(self):
        self._pack.on_response = self.__on_parse
        self._microphone = {"maximum":0,"average":0}
        self.subscribe_microphone()

    def __on_parse(self, pack):
        try:
            ret = eval("".join([ chr(i) for i in pack.data[3:len(pack.data)]]))
            if not ret['ret'] is None:
                self._is_touched = ret['ret']
                if not self._callback==None:
                    self._callback(ret["ret"])
        except:
            print("error")
        else:
            pass

    @property
    def loudness(self):
        return self._microphone['average']

    @property
    def maximum(self):
        return self._microphone['maximum']

    def on_subscribe_response(self,pack):
        self._microphone[pack.value_name] = pack.subscribe_value

    def subscribe_microphone(self):
        pack = HalocodePackData()
        pack.value_name = "average"
        pack.type = HalocodePackData.TYPE_SCRIPT
        pack.mode = HalocodePackData.TYPE_RUN_WITH_RESPONSE
        pack.on_response = self.on_subscribe_response
        pack.script = "subscribe.add_item({0}, microphone.get_loudness, ())"
        self.subscribe(pack)
        pack = HalocodePackData()
        pack.value_name = "maximum"
        pack.type = HalocodePackData.TYPE_SCRIPT
        pack.mode = HalocodePackData.TYPE_RUN_WITH_RESPONSE
        pack.on_response = self.on_subscribe_response
        pack.script = 'subscribe.add_item({0}, microphone.get_loudness, ("maximum",))'
        self.subscribe(pack)