# -*- coding: utf-8 -*
import struct
import time
from time import ctime, sleep
import makeblock.utils
from makeblock.protocols.PackData import NeuronPackData

class _BaseModule:
    def __init__(self,board,port=0,slot=0):
        self._pack = NeuronPackData()
        self._pack.idx = 1
        self.setup(board,port,slot)
        
    def _callback(self,data):
        pass

    def setup(self,board,port,slot):
        self._pack.port = port
        self._pack.slot = slot
        self._board = board
        self._init_module()
    
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

    def subscribe(self,pack):
        self._board.request(pack)

class RGBLed(_BaseModule):
    def _init_module(self):
        pass
    
    def set_color(self,red,green,blue):
        self._pack.service = 0x73
        self._pack.subservice = 0x8
        self._pack.data = [0x2,((self._pack.port<<4)+self._pack.slot)&0x7f]
        self._pack.data.extend(makeblock.utils.ushort2bits(red))
        self._pack.data.extend(makeblock.utils.ushort2bits(green))
        self._pack.data.extend(makeblock.utils.ushort2bits(blue))
        self.call(self._pack)

    def set_color_at(self,index,red,green,blue):
        self._pack.service = 0x73
        self._pack.subservice = 0x8
        self._pack.data = [0x2,((self._pack.port<<4)+self._pack.slot)&0x7f,index]
        self._pack.data.extend(makeblock.utils.ushort2bits(red))
        self._pack.data.extend(makeblock.utils.ushort2bits(green))
        self._pack.data.extend(makeblock.utils.ushort2bits(blue))
        self.call(self._pack)

    def set_colors(self,colors):
        self._pack.service = 0x73
        self._pack.subservice = 0x8
        self._pack.data = [0x2,((self._pack.port<<4)+self._pack.slot)&0x7f]
        for i in range(len(colors)):
            self._pack.data.extend(makeblock.utils.ushort2bits(colors[i]))
        
        # makeblock.utils.print_hex(self._pack.to_buffer())
        
        self.call(self._pack)

class ButtonOnBoard(_BaseModule):
    def _init_module(self):
        self._is_pressed = False
        self._pack.service = 0x73
        self._pack.subservice = 0x23 
        self._pack.on_response = self.__on_response
        self._callback = None
        self.subscribe_pressed()
    
    def __on_subscribe_response(self,pack):
        self._is_pressed = pack.data[0]==0

    def __on_response(self,pack):
        self._is_pressed = pack.data[0]==0
        if not self._callback is None:
            self._callback(self._is_pressed)

    @property
    def is_pressed(self):
        return self._is_pressed

    def subscribe_pressed(self):
        pack = NeuronPackData()
        pack.on_response = self.__on_subscribe_response
        pack.idx = self._pack.idx
        pack.service = self._pack.service
        pack.subservice = self._pack.subservice
        pack.data = [0x2,0]
        self.subscribe(pack)

    def request_pressed(self,callback):
        self._callback = callback
        self._pack.data = [0x1,0]
        self.request(self._pack)


class Ultrasonic(_BaseModule):
    def _init_module(self):
        self._distance = 0
        self._pack.service = 0x73
        self._pack.subservice = 0x1 
        self._pack.on_response = self.__on_response
        self._callback = None
        self.subscribe_distance()
    
    def __on_subscribe_response(self,pack):
        self._distance = int(makeblock.utils.bits2float(pack.data)*10)/10

    def __on_response(self,pack):
        self._distance = int(makeblock.utils.bits2float(pack.data)*10)/10
        if not self._callback is None:
            self._callback(self._distance)

    @property
    def distance(self):
        return self._distance

    def subscribe_distance(self):
        pack = NeuronPackData()
        pack.on_response = self.__on_subscribe_response
        pack.idx = self._pack.idx
        pack.service = self._pack.service
        pack.subservice = self._pack.subservice
        pack.data = [0x2,((self._pack.port<<4)+self._pack.slot)&0xff]
        pack.data.extend(makeblock.utils.long2bits(30))
        self.subscribe(pack)

    def request_distance(self,callback):
        self._callback = callback
        self._pack.data = [0x1,((self._pack.port<<4)+self._pack.slot)&0xff]
        self.request(self._pack)