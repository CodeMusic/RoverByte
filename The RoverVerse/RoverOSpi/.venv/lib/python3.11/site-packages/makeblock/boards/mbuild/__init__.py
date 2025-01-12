# -*- coding: utf-8 -*
from makeblock.modules.mbuild import *
import makeblock.protocols as Protocols
from ...boards.base import _BaseBoard
from ...protocols.PackData import NeuronPackData
from ...comm.SerialPort import SerialPort
from ...comm import mlink
from time import sleep
MODE_REQUEST = 0
MODE_CHANGE = 1
MODE_PERIOD = 2
GESTURE_NONE = 0
GESTURE_WAVING = 1
GESTURE_MOVING_UP = 2
GESTURE_MOVING_DOWN = 3
C3 = 131
D3 = 147
E3 = 165
F3 = 174
G3 = 196
A3 = 220
B3 = 247
C4 = 261
D4 = 293
E4 = 329
F4 = 349
G4 = 392
A4 = 440
B4 = 493
C5 = 523
D5 = 587
E5 = 659
F5 = 698
G5 = 784
A5 = 880
B5 = 987
def connect(device=None,channel=None):
    return __mBuild(device or channel)

create = connect

class __mBuild(_BaseBoard):
    _device = None
    def __init__(self,device=None):
        self._type = _BaseBoard.mBuild
        if not device is None:
            self._device = device
            super().__init__(_BaseBoard.mBuild,device)
            self.broadcast()

    def broadcast(self):
        self.call(NeuronPackData.broadcast())

    _speaker = {}
    def Speaker(self,idx=1):
        if not idx in self._speaker:
            self._speaker[idx] = Speaker(self,idx)
        return self._speaker[idx]

    @property
    def speaker(self):
        return self.Speaker()

    _ledmatrix = {}
    def LedMatrix(self,idx=1):
        if not idx in self._ledmatrix:
            self._ledmatrix[idx] = LedMatrix(self,idx)
        return self._ledmatrix[idx]

    @property
    def ledmatrix(self):
        return self.LedMatrix(self)

    _rgbled = {}
    def RGBLed(self,idx=1):
        if not idx in self._rgbled:
            self._rgbled[idx] = RGBLed(self,idx)
        return self._rgbled[idx]

    @property
    def rgbled(self):
        return self.RGBLed(self)

    def LedStrip(self,idx=1):
        return LedStrip(self,idx)

    _piano = {}
    def Piano(self,idx=1):
        if not idx in self._piano:
            self._piano[idx] = FingertipPiano(self,idx)
        return self._piano[idx]

    @property
    def piano(self):
        return self.Piano()

    _ultrasonic = {}
    def Ultrasonic(self,idx=1):
        if not idx in self._ultrasonic:
            self._ultrasonic[idx] = Ultrasonic(self,idx)
        return self._ultrasonic[idx]

    @property
    def ultrasonic(self):
        return self.Ultrasonic()

    _button = {}
    def Button(self,idx=1):
        if not idx in self._button:
            self._button[idx] = Button(self,idx)
        return self._button[idx]

    @property
    def button(self):
        return self.Button()

    _slider = {}
    def Slider(self,idx=1):
        if not idx in self._slider:
            self._slider[idx] = Slider(self,idx)
        return self._slider[idx]

    @property
    def slider(self):
        return self.Slider()

    _joystick = {}
    def Joystick(self,idx=1):
        if not idx in self._joystick:
            self._joystick[idx] = Joystick(self,idx)
        return self._joystick[idx]
        
    @property
    def joystick(self):
        return self.Joystick()

    _soilmoisture = {}
    def SoilMoisture(self,idx=1):
        if not idx in self._soilmoisture:
            self._soilmoisture[idx] = SoilMoisture(self,idx)
        return self._soilmoisture[idx]

    @property
    def soilmoisture(self):
        return self.SoilMoisture()

    _laserranging = {}
    def LaserRanging(self,idx=1):
        if not idx in self._laserranging:
            self._laserranging[idx] = LaserRanging(self,idx)
        return self._laserranging[idx]

    @property
    def laserranging(self):
        return self.LaserRanging()

    _flame = {}
    def Flame(self,idx=1):
        if not idx in self._flame:
            self._flame[idx] = Flame(self,idx)
        return self._flame[idx]

    @property
    def flame(self):
        return self.Flame()

    _touch = {}
    def Touch(self,idx=1):
        if not idx in self._touch:
            self._touch[idx] = Touch(self,idx)
        return self._touch[idx]

    @property
    def touch(self):
        return self.Touch()

    _sound = {}
    def Sound(self,idx=1):
        if not idx in self._sound:
            self._sound[idx] = Sound(self,idx)
        return self._sound[idx]

    @property
    def sound(self):
        return self.Sound()

    _light = {}
    def Light(self,idx=1):
        if not idx in self._light:
            self._light[idx] = Light(self,idx)
        return self._light[idx]

    @property
    def light(self):
        return self.Light()
    
    _pirmotion = {}
    def PIRMotion(self,idx=1):
        if not idx in self._pirmotion:
            self._pirmotion[idx] = PIRMotion(self,idx)
        return self._pirmotion[idx]

    @property
    def pirmotion(self):
        return self.PIRMotion()

    _magnetic = {}
    def Magnetic(self,idx=1):
        if not idx in self._magnetic:
            self._magnetic[idx] = Magnetic(self,idx)
        return self._magnetic[idx]

    @property
    def magnetic(self):
        return self.Magnetic()

    _angle = {}
    def Angle(self,idx=1):
        if not idx in self._angle:
            self._angle[idx] = Angle(self,idx)
        return self._angle[idx]

    @property
    def angle(self):
        return self.Angle()

    _motion = {}
    def Motion(self,idx=1):
        if not idx in self._motion:
            self._motion[idx] = Motion(self,idx)
        return self._motion[idx]

    @property
    def motion(self):
        return self.Motion()

    _servo = {}
    def Servo(self,idx=1):
        if not idx in self._servo:
            self._servo[idx] = Servo(self,idx)
        return self._servo[idx]

    @property
    def servo(self):
        return self.Servo()

    _dcmotor = {}
    def DCMotor(self,idx=1):
        if not idx in self._dcmotor:
            self._dcmotor[idx] = DCMotor(self,idx)
        return self._dcmotor[idx]

    @property
    def dcmotor(self):
        return self.DCMotor()

    _encodermotor = {}
    def EncoderMotor(self,idx=1):
        if not idx in self._encodermotor:
            self._encodermotor[idx] = EncoderMotor(self,idx)
        return self._encodermotor[idx]

    @property
    def encodermotor(self):
        return self.EncoderMotor()

    _color = {}
    def Color(self,idx=1):
        if not idx in self._color:
            self._color[idx] = Color(self,idx)
        return self._color[idx]

    @property
    def color(self):
        return self.Color()

    _gpio = {}
    def GPIO(self,idx=1):
        if not idx in self._gpio:
            self._gpio[idx] = GPIO(self,idx)
        return self._gpio[idx]

    @property
    def gpio(self):
        return self.GPIO()

    _powermanager = {}
    def PowerManager(self,idx=1):
        if not idx in self._powermanager:
            self._powermanager[idx] = PowerManager(self,idx)
        return self._powermanager[idx]

    @property
    def powermanager(self):
        return self.PowerManager()

    _infrarer = {}
    def Infrarer(self,idx=1):
        if not idx in self._infrarer:
            self._infrarer[idx] = Infrarer(self,idx)
        return self._infrarer[idx]

    @property
    def infrarer(self):
        return self.Infrarer()

    _temp = {}
    def Temperature(self,idx=1):
        if not idx in self._temp:
            self._temp[idx] = Temperature(self,idx)
        return self._temp[idx]

    @property
    def temperature(self):
        return self.Temperature()

    _humiture = {}
    def Humiture(self,idx=1):
        if not idx in self._humiture:
            self._humiture[idx] = Humiture(self,idx)
        return self._humiture[idx]

    @property
    def humiture(self):
        return self.Humiture()

    _extdcmotor = {}
    def ExtDCMotor(self,idx=1):
        if not idx in self._extdcmotor:
            self._extdcmotor[idx] = ExtDCMotor(self,idx)
        return self._extdcmotor[idx]

    @property
    def extdcmotor(self):
        return self.ExtDCMotor()

    _smartservo = {}
    def SmartServo(self,idx=1):
        if not idx in self._smartservo:
            self._smartservo[idx] = SmartServo(self,idx)
        return self._smartservo[idx]

    @property
    def smartservo(self):
        return self.SmartServo()
    
    