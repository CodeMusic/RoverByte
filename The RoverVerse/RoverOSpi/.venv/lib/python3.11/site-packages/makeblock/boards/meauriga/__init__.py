# -*- coding: utf-8 -*
from makeblock.modules.rj25 import *
from makeblock.boards.megapi import __MegaPi
from makeblock.boards.base import _BaseBoard

PORT1 = 1
PORT2 = 2
PORT3 = 3
PORT4 = 4
PORT5 = 5
PORT6 = 6
PORT7 = 7
PORT8 = 8
SLOT1 = 1
SLOT2 = 2


def connect(device=None, channel=None):
    return __MeAuriga(device or channel)

create = connect

class __MeAuriga(__MegaPi):

    _led = None
    _light = None
    _sound = None
    _buzzer = None
    _gyro = None
    _temperature = None

    LIGHT_SENSOR_1 = 1
    LIGHT_SENSOR_2 = 2
    def __init__(self,device=None):
        self._type = _BaseBoard.MeAuriga
        if not device is None:
            super().__init__(_BaseBoard.MeAuriga,device)

    def set_color(self,index,red,green,blue):
        return self.rgbled.set_color(index,red,green,blue)

    def set_tone(self,hz,ms=0):
        return self.buzzer.set_tone(hz,ms)

    def get_temperature(self,callback=None):
        return self.temperature.get_temperature(callback)

    def get_lightness(self,index,callback=None):
        return self.light.read(callback,13-index)

    def get_roll(self,callback=None):
        return self.gyro.get_x(callback)

    def get_pitch(self,callback=None):
        return self.gyro.get_y(callback)

    def get_yaw(self,callback=None):
        return self.gyro.get_z(callback)
        
    @property
    def temperature(self):
        if self._temperature is None:
            self._temperature = TemperatureOnBoard(self,13)
        return self._temperature

    @property
    def rgbled(self):
        if self._led is None:
            self._led = RGBLed(self,0,2)
        return self._led

    @property
    def light(self):
        if self._light is None:
            self._light = Light(self)
        return self._light
    
    @property
    def gyro(self):
        if self._gyro is None:
            self._gyro = Gyro(self,1)
        return self._gyro
    
    @property
    def buzzer(self):
        if self._buzzer is None:
            self._buzzer = Buzzer(self,45)
        return self._buzzer