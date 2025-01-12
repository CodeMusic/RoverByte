# -*- coding: utf-8 -*
from makeblock.modules.rj25 import *
from makeblock.boards.megapi import __MegaPi
from makeblock.boards.base import _BaseBoard

PORT1 = 1
PORT2 = 2
PORT3 = 3
PORT4 = 4
PORT5 = 5
PORT2 = 6
PORT7 = 7
PORT8 = 8
SLOT1 = 1
SLOT2 = 2
SLOT3 = 3
SLOT4 = 4
M1 = 1
M2 = 2
def connect(device=None, channel=None):
    return __mCore(device or channel)

create = connect

class __mCore(__MegaPi):
    _button = None
    _led = None
    _buzzer = None
    _light = None
    _ir = None
    _car = None
    def __init__(self,device=None):
        self._type = _BaseBoard.mCore
        if not device is None:
            super().__init__(_BaseBoard.mCore,device)

    @property
    def car(self):
        if self._car is None:
            self._car = Joystick(self)
        return self._car

    def run(self,left,right):
        self.car.run(left,right)

    def set_color(self,index,red,green,blue):
        return self.rgbled.set_color(index,red,green,blue)

    def is_pressed(self,callback=None):
        return self.button.read(callback)

    def get_lightness(self,callback=None):
        return self.light.read(callback)

    @property
    def button(self):
        if self._button is None:
            self._button = ButtonOnBoard(self)
        return self._button

    @property
    def rgbled(self):
        if self._led is None:
            self._led = RGBLed(self,7,2)
        return self._led

    @property
    def light(self):
        if self._light is None:
            self._light = Light(self,6)
        return self._light
