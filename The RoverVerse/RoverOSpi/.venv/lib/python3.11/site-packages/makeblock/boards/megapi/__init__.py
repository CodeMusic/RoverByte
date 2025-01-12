# -*- coding: utf-8 -*
from ...modules.rj25 import *
from ...boards.base import _BaseBoard

def connect(device=None,channel=None):
    return __MegaPi(device or channel)

create = connect

class __MegaPi(_BaseBoard):

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
    SLOT3 = 3
    SLOT4 = 4
    M9 = 9
    M10 = 10
    M11 = 11
    M12 = 12
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
    def __init__(self,device=None):
        self._type = _BaseBoard.MegaPi
        if not device is None:
            super().__init__(_BaseBoard.MegaPi,device)
        
    _steppers = {}
    def StepperMotor(self,slot):
        if not slot in self._steppers:
            self._steppers[slot] = StepperMotor(self,slot)
        return self._steppers[slot]

    @property
    def steppermotor(self):
        return self.StepperMotor(1)

    _encoders = {}
    def EncoderMotor(self,slot):
        if not slot in self._encoders:
            self._encoders[slot] = EncoderMotor(self,slot)
        return self._encoders[slot]

    @property
    def encodermotor(self):
        return self.EncoderMotor(1)

    _servos = {}
    def Servo(self,port,slot=0):
        idx = (port<<8)+slot
        if not idx in self._servos:
            self._servos[idx] = Servo(self,port,slot)
        return self._servos[idx]

    @property
    def servo(self):
        return self.Servo(self,0,0)

    _dcmotors = {}
    def DCMotor(self,port,slot=1):
        if not port in self._dcmotors:
            self._dcmotors[port*2+slot] = DCMotor(self,port,slot)
        return self._dcmotors[port*2+slot]

    @property
    def dcmotor(self):
        return self.DCMotor(1)

    _rgbleds = {}
    def RGBLed(self,port=0,slot=2):
        idx = (port<<8)+slot
        if not idx in self._rgbleds:
            self._rgbleds[idx] = RGBLed(self,port,slot)
        return self._rgbleds[idx]

    @property
    def rgbled(self):
        return self.RGBLed(3)

    _sevens = {}
    def SevenSegmentDisplay(self,port):
        if not port in self._sevens:
            self._sevens[port] = SevenSegmentDisplay(self,port)
        return self._sevens[port]

    @property
    def sevensegmentdisplay(self):
        return self.SevenSegmentDisplay(0,0)

    _ledmatrix = {}
    def LedMatrix(self,port):
        if not port in self._ledmatrix:
            self._ledmatrix[port] = LedMatrix(self,port)

    @property
    def ledmatrix(self):
        return self.LedMatrix(0)

    _shutters = {}
    def Shutter(self,port):
        if not port in self._shutters:
            self._shutters[port] = DSLRShutter(self,port)
        return self._shutters[port]

    @property
    def shutter(self):
        return self.DSLRShutter(0)
    
    _irs = {}
    def InfraredReceiver(self,port):
        if not port in self._irs:
            self._irs[port] = InfraredReceiver(self,port)
        return self._irs[port]

    @property
    def infraredreceiver(self):
        return self.InfraredReceiver(0)

    _ultrasonics = {}
    def Ultrasonic(self,port):
        if not port in self._ultrasonics:
            self._ultrasonics[port] = Ultrasonic(self,port)
        return self._ultrasonics[port]

    @property
    def ultrasonic(self):
        return self.Ultrasonic(self,0)

    _buttons = {}
    def Button(self,port):
        if not port in self._buttons:
            self._buttons[port] = Button(self,port)
        return self._buttons[port]

    @property
    def button(self):
        return self.Button(0)

    _linefollowers = {}
    def LineFollower(self,port):
        if not port in self._linefollowers:
            self._linefollowers[port] = LineFollower(self,port)
        return self._linefollowers[port]

    @property
    def linefollower(self):
        return self.LineFollower(0)

    _limitswitches = {}
    def LimitSwitch(self,port,slot=2):
        idx = (port<<8)+slot
        if not idx in self._linefollowers:
            self._limitswitches[idx] = LimitSwitch(self,port,slot)
        return self._limitswitches[idx]

    @property
    def limitswitch(self):
        return self.LimitSwitch(0,0)

    _pirmotions = {}
    def PIRMotion(self,port):
        idx = (port<<8)
        if not idx in self._pirmotions:
            self._pirmotions[idx] = PIRMotion(self,port)
        return self._pirmotions[idx]

    @property
    def pirmotion(self):
        return self.PIRMotion(0)

    _lights = {}
    def Light(self,port):
        idx = (port<<8)
        if not idx in self._lights:
            self._lights[idx] = Light(self,port)
        return self._lights[idx]
    
    @property
    def light(self):
        return self.Light(0)

    _sounds = {}
    def Sound(self,port):
        idx = (port<<8)
        if not idx in self._sounds:
            self._sounds[idx] = Sound(self,port)
        return self._sounds[idx]

    @property
    def sound(self):
        return self.Sound(0)

    _potentiometers = {}
    def Potentiometer(self,port):
        idx = (port<<8)
        if not idx in self._potentiometers:
            self._potentiometers[idx] = Potentiometer(self,port)
        return self._potentiometers[idx]

    @property
    def potentiometer(self):
        return self.Potentiometer(0)

    _joysticks = {}
    def Joystick(self,port):
        idx = (port<<8)
        if not idx in self._joysticks:
            self._joysticks[idx] = Joystick(self,port)
        return self._joysticks[idx]

    @property
    def joystick(self):
        return self.Joystick(0)

    _gyro = None
    def Gyro(self):
        if self._gyro is None:
            self._gyro = Gyro(self)
        return self._gyro

    @property
    def gyro(self):
        return self.Gyro()

    _compass = None
    def Compass(self):
        if self._compass is None:
            self._compass = Compass(self)
        return self._compass

    @property
    def compass(self):
        return self.Compass()

    _temperatures = {}
    def Temperature(self,port,slot=2):
        idx = (port<<8)+slot
        if not idx in self._temperatures:
            self._temperatures[idx] = Temperature(self,port,slot)
        return self._temperatures[idx]

    @property
    def temperature(self):
        return self.Temperature(0,0)

    _humitures = {}
    def Humiture(self,port):
        idx = (port<<8)
        if not idx in self._humitures:
            self._humitures[idx] = Humiture(self,port)
        return self._humitures[idx]

    @property
    def humiture(self):
        return self.Humiture(0)

    _flames = {}
    def Flame(self,port):
        idx = (port<<8)
        if not idx in self._flames:
            self._flames[idx] = Flame(self,port)
        return self._flames[idx]

    @property
    def flame(self):
        return self.Flame(0)

    _gases = {}
    def Gas(self,port):
        idx = (port<<8)
        if not idx in self._gases:
            self._gases[idx] = Gas(self,port)
        return self._gases[idx]

    @property
    def gas(self):
        return self.Gas(0)

    _touches = {}
    def Touch(self,port):
        idx = (port<<8)
        if not idx in self._touches:
            self._touches[idx] = Touch(self,port)
        return self._touches[idx]

    @property
    def touch(self):
        return Touch(self,0)

    _colors = {}
    def Color(self,port):
        idx = (port<<8)
        if not idx in self._colors:
            self._colors[idx] = Color(self,port)
        return self._colors[idx]

    @property
    def color(self):
        return self.Color(0)

    _pin = None
    def Pin(self,port=0):
        if self._pin is None:
            self._pin = Pin(self)
        return self._pin

    @property
    def pin(self):
        return self.Pin()