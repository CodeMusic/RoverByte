# -*- coding: utf-8 -*
import struct
import time
from time import ctime, sleep
import makeblock.utils
from makeblock.protocols.PackData import NeuronPackData

class _BaseModule:
    def __init__(self,board,idx=1,mode=1,period=0):
        self._pack = None
        self.setup(board,idx,mode,period)
        
    def _callback(self,data):
        pass

    def setup(self,board,idx=1,mode=1,period=0):
        self._init_module()
        self._board = board
        self._mode = mode
        self._pack.idx = idx
        if self._pack.type==NeuronPackData.TYPE_SENSOR:
            self._pack.data = [0x7f,mode]
            self._pack.data.extend(makeblock.utils.long2bits(period))
            self.request(self._pack)
    
    def _init_module(self):
        pass

    def force_update(self):
        self._pack.data = [0x1]
        self.request(self._pack)

    def request(self,pack):
        self._board.remove_response(pack)
        self._board.request(pack)

    def call(self,pack):
        self._board.call(pack)

class Temperature(_BaseModule):
    def _init_module(self):
        self._temperature = 0
        self._pack = NeuronPackData()
        self._pack.type = NeuronPackData.TYPE_SENSOR
        self._pack.service = 0x63
        self._pack.subservice = 0x1
        self._pack.on_response = self.__on_parse
        
    def __on_parse(self, pack):
        self._temperature = makeblock.utils.bits2float(pack.data[1:6])
        self._callback(self._temperature)

    def on_change(self,callback):
        self._callback = callback

    @property
    def temperature(self):
        return self._temperature

class Humiture(_BaseModule):
    def _init_module(self):
        self._pack = NeuronPackData()
        self._pack.type = NeuronPackData.TYPE_SENSOR
        self._pack.service = 0x63
        self._pack.subservice = 0x7
        self._pack.on_response = self.__on_parse
        self._status = {"temp":0,"hum":0}
        
    def __on_parse(self, pack):
        self._status["temp"] = makeblock.utils.bits2short(pack.data[1:3])
        self._status["hum"] = pack.data[3]
        self._callback(self._status)

    def on_change(self,callback):
        self._callback = callback

    @property
    def temperature(self):
        return self._status["temp"]
    
    @property
    def humiture(self):
        return self._status["hum"]

class Ultrasonic(_BaseModule):
    def _init_module(self):
        self._distance = 0
        self._pack = NeuronPackData()
        self._pack.type = NeuronPackData.TYPE_SENSOR
        self._pack.service = 0x63
        self._pack.subservice = 0x3
        self._pack.on_response = self.__on_parse
        
    def __on_parse(self, pack):
        self._distance = makeblock.utils.bits2float(pack.data[1:6])
        self._callback(self._distance)

    def on_change(self,callback):
        self._callback = callback

    @property
    def distance(self):
        return self._distance
        
class LineFollower(_BaseModule):
    def _init_module(self):
        self._pack = NeuronPackData()
        self._pack.type = NeuronPackData.TYPE_SENSOR
        self._pack.service = 0x63
        self._pack.subservice = 0x4
        self._pack.on_response = self.__on_parse
        self._status = 0
        
    def __on_parse(self, pack):
        self._status = pack.data[1]
        self._callback(self._status)

    def on_change(self,callback):
        self._callback = callback

    @property
    def status(self):
        return self._status

class RGBLed(_BaseModule):
    def _init_module(self):
        self._pack = NeuronPackData()
    
    def set_color(self,red,green,blue):
        self._pack.service = 0x65
        self._pack.subservice = 0x2
        self._pack.data = [0x1]
        self._pack.data.extend(makeblock.utils.ushort2bits(red))
        self._pack.data.extend(makeblock.utils.ushort2bits(green))
        self._pack.data.extend(makeblock.utils.ushort2bits(blue))
        self.call(self._pack)
    
class LedStrip(_BaseModule):
    def _init_module(self):
        self._pack = NeuronPackData()
        self._pack.service = 0x65
        self._pack.subservice = 0x3
    
    def set_color(self,index,red,green,blue):
        self._pack.data = [0x1,index]
        self._pack.data.extend(makeblock.utils.ushort2bits(red))
        self._pack.data.extend(makeblock.utils.ushort2bits(green))
        self._pack.data.extend(makeblock.utils.ushort2bits(blue))
        self.call(self._pack)

class LedMatrix(_BaseModule):
    def _init_module(self):
        self._pack = NeuronPackData()
        self._pack.service = 0x65
        self._pack.subservice = 0x4
    
    def set_pixel(self,index,red,green,blue):
        self._pack.data = [0x2,index]
        self._pack.data.extend(makeblock.utils.ushort2bits(red))
        self._pack.data.extend(makeblock.utils.ushort2bits(green))
        self._pack.data.extend(makeblock.utils.ushort2bits(blue))
        self.call(self._pack)
    
    def set_pixels(self,bits,red,green,blue):
        self._pack.data = [0x1]
        for i in range(2):
            self._pack.data.extend(makeblock.utils.long2bits(bits[i]))
        self._pack.data.extend(makeblock.utils.ushort2bits(red))
        self._pack.data.extend(makeblock.utils.ushort2bits(green))
        self._pack.data.extend(makeblock.utils.ushort2bits(blue))
        self.call(self._pack)

class Servo(_BaseModule):
    BOTH_SERVOS = 1
    LEFT_SERVO = 2
    RIGHT_SERVO = 3
    def _init_module(self):
        self._pack = NeuronPackData()
        self._pack.service = 0x62
        self._pack.subservice = 0x3
    
    def set_angle(self,index,angle):
        self._pack.data = [index]
        self._pack.data.extend(makeblock.utils.ushort2bits(angle))
        self.call(self._pack)