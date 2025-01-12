# -*- coding: utf-8 -*
import struct
from time import ctime, sleep
import json
import makeblock.utils
from makeblock.protocols.PackData import HalocodePackData

class _BaseModule:
    def __init__(self,board):
        self._pack = None
        self.setup(board)
        
    def _callback(self,data):
        pass

    def setup(self,board):
        self._board = board
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

class RGBLed(_BaseModule):
    def _init_module(self):
        pass

class Speaker(_BaseModule):
    def _init_module(self):
        pass

class LedMatrix(_BaseModule):
    def _init_module(self):
        pass

class IrRemote(_BaseModule):
    def _init_module(self):
        pass

class Rocky(_BaseModule):
    def _init_module(self):
        self._pack = HalocodePackData()
        self._pack.type = HalocodePackData.TYPE_SCRIPT

    def run(self,left,right):
        self.call(self._pack)

    def on_light(self):
        pass

    def on_color(self):
        pass

class Gamepad(_BaseModule):
    def _init_module(self):
        pass

class Gyro(_BaseModule):
    def _init_module(self):
        pass

class Power(_BaseModule):
    def _init_module(self):
        pass
      
class Potentiometer(_BaseModule): 
    def _init_module(self):
        pass

class Sound(_BaseModule):
    def _init_module(self):
        self._pack = HalocodePackData()
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

    def on_pressed(self,callback):
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