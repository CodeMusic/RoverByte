# -*- coding: utf-8 -*
from time import sleep
from ...modules.halocode import *
from ...boards.base import _BaseBoard
from ...protocols.PackData import HalocodePackData
from ...boards.mbuild import __mBuild as mBuild
from ...comm.SerialPort import SerialPort
from ...comm import mlink
MODE_REQUEST = 0
MODE_CHANGE = 1
MODE_PERIOD = 2
board = None

def connect(device=None,channel=None):
    return __Halocode(device or channel)
  
create = connect
  
class __Halocode(_BaseBoard):
    def __init__(self,device=None):
        self._led = None
        self._button = None
        self._pin0 = None
        self._pin1 = None
        self._pin2 = None
        self._pin3 = None
        self._motion = None
        self._microphone = None
        self._type = _BaseBoard.Halocode
        self._mbuild = None
        if not device is None:
            super().__init__(_BaseBoard.Halocode,device)
            self.broadcast()

    def setTransferMode(self):
        self.call(HalocodePackData.broadcast())
        sleep(0.5)
        self.call(HalocodePackData.repl_mode())
        sleep(0.5)
        self.repl('import communication')
        self.repl('communication.bind_passthrough_channels("uart0", "uart1")')

    def broadcast(self):
        pack = HalocodePackData()
        pack.type = HalocodePackData.TYPE_SCRIPT
        pack.mode = HalocodePackData.TYPE_RUN_WITH_RESPONSE
        pack.script = ""
        while not self.protocol.ready:
            self.call(pack)
            sleep(0.3)
        self.setTransferMode()
    
    def set_led(self,idx,red,green,blue):
        '''
        :description: set rgb led's color on board

        :example:

        .. code-block:: python
            :linenos:

            while True:
                board.set_led(0,30,0,0)
                sleep(0.5)
                board.set_led(0,0,30,0)
                sleep(0.5)
                board.set_led(0,0,0,30)
                sleep(0.5)
        ''' 
        if self._led is None:
            self._led = Led(self)
        self._led.set_color(idx,red,green,blue)

    def set_leds(self,red,green,blue):
        '''
        :description: set rgb leds' colors on board

        :example:

        .. code-block:: python
            :linenos:

            while True:
                board.set_leds(30,0,0)
                sleep(0.5)
                board.set_leds(0,30,0)
                sleep(0.5)
                board.set_leds(0,0,30)
                sleep(0.5)
        ''' 
        if self._led is None:
            self._led = Led(self)
        self._led.set_colors(red,green,blue)

    def set_full_leds(self,colors):
        if self._led is None:
            self._led = Led(self)
        self._led.set_full_colors(colors)

    def is_pressed(self):
        '''
        :description: whether Button on board is pressed

        :example:

        .. code-block:: python
            :linenos:

            while True:
                print(board.is_pressed)
                sleep(0.1)
        '''     
        if self._button is None:
            self._button = Button(self)
        return self._button.is_pressed

    def _init_pin(self,pin):
        if pin==0:
            if self._pin0 is None:
                self._pin0 = Pin(self,0)
            return self._pin0
        elif pin==1:
            if self._pin1 is None:
                self._pin1 = Pin(self,1)
            return self._pin1
        elif pin==2:
            if self._pin2 is None:
                self._pin2 = Pin(self,2)
            return self._pin2
        elif pin==3:
            if self._pin3 is None:
                self._pin3 = Pin(self,3)
            return self._pin3

    def is_touched(self,pin_number):
        '''
        :description: set pin input as touchpad

        :param pin_number: range:0~3
        :type pin_number: int
        
        :example:

        .. code-block:: python
            :linenos:

            while True:
                board.is_touched(2)
                sleep(1)
        '''     
        pin = self._init_pin(pin_number)
        return pin.is_touched

    def write_digital(self,pin_number,level):
        '''
        :description: set pin output as digital pin

        :example:

        .. code-block:: python
            :linenos:

            while True:
                board.write_digital(2,0)
                sleep(1)
                board.write_digital(2,1)
                sleep(1)
        '''     
        pin = self._init_pin(pin_number)
        pin.write_digital(pin_number,level)

    def write_pwm(self,pin_number,pwm):
        '''
        :description: set pin output as pwm pin

        :param pin_number: range:0~3
        :type pin_number: int
        :param pwm: range:0~255
        :type pwm: int
        
        :example:

        .. code-block:: python
            :linenos:

            while True:
                board.write_pwm(2,100)
                sleep(1)
                board.write_pwm(2,0)
                sleep(1)
        '''
        pin = self._init_pin(pin_number)
        pin.write_pwm(pin_number,pwm)

    def write_servo(self,pin_number,angle):
        '''
        :description: set pin output as pulse pin for servo driving

        :param pin_number: range:0~3
        :type pin_number: int
        :param pwm: range:0~180
        :type pwm: int
        
        :example:

        .. code-block:: python
            :linenos:

            while True:
                board.write_servo(2,120)
                sleep(1)
                board.write_servo(2,60)
                sleep(1)
        '''
        pin = self._init_pin(pin_number)
        pin.servo_write(pin_number,angle)

    def read_digital(self,pin_number,callback):
        '''
        :description: set pin input as digital pin

        :param pin_number: range:0~3
        :type pin_number: int
        :param callback: callback
        :type callback: function
        
        :example:

        .. code-block:: python
            :linenos:

            def on_read(value):
                print("level:",value)
            while True:
                board.read_digital(2,on_read)
                sleep(1)
        '''
        pin = self._init_pin(pin_number)
        pin.read_digital(callback)

    def read_analog(self,pin_number,callback):
        '''
        :description: set pin input as analog pin

        :param pin_number: range:0~3
        :type pin_number: int
        :param callback: callback
        :type callback: function
        
        :example:

        .. code-block:: python
            :linenos:

            def on_read(value):
                print("value:",value)
            while True:
                board.read_analog(2,on_read)
                sleep(1)
        '''
        pin = self._init_pin(pin_number)
        pin.read_analog(callback)

    def is_shaked(self):
        return self.is_shaking
        
    def is_shaking(self):
        '''
        :description: whether the halocode is shaked
        
        :example:

        .. code-block:: python
            :linenos:

            while True:
                print("is shaking:",board.is_shaking)
                sleep(0.1)
        '''
        if self._motion is None:
            self._motion = Motion(self)
        return self._motion.is_shaking
    
    def roll(self):
        '''
        :description: the halocode's roll degree
        
        :example:

        .. code-block:: python
            :linenos:

            while True:
                print("roll:",board.roll)
                sleep(0.1)
        '''
        if self._motion is None:
            self._motion = Motion(self)
        return self._motion.roll
    
    def yaw(self):
        '''
        :description: the halocode's yaw degree
        
        :example:

        .. code-block:: python
            :linenos:

            while True:
                print("yaw:",board.yaw)
                sleep(0.1)
        '''
        if self._motion is None:
            self._motion = Motion(self)
        return self._motion.yaw
    
    def pitch(self):
        '''
        :description: the halocode's pitch degree
        
        :example:

        .. code-block:: python
            :linenos:

            while True:
                print("pitch:",board.pitch)
                sleep(0.1)
        '''
        if self._motion is None:
            self._motion = Motion(self)
        return self._motion.pitch

    def shake_level(self):
        '''
        :description: the strength level of shaking
        
        :example:

        .. code-block:: python
            :linenos:

            while True:
                print("shake_level:",board.shake_level)
                sleep(0.1)
        '''
        if self._motion is None:
            self._motion = Motion(self)
        return self._motion.shake_strength
    
    def loudness(self):
        '''
        :description: loudness from the microphone on board
        
        :example:

        .. code-block:: python
            :linenos:

            while True:
                print("loudness:",board.loudness)
                sleep(0.1)
        '''
        if self._microphone is None:
            self._microphone = Microphone(self)
        return self._microphone.loudness
    
    def is_pitch_up(self):
        '''
        :description: whether motion status is pitch up
        
        :example:

        .. code-block:: python
            :linenos:

            while True:
                print("is_pitch_up:",board.is_pitch_up)
                sleep(0.1)
        '''
        if self._motion is None:
            self._motion = Motion(self)
            return 0
        return self._motion.pitch<-15
    
    def is_pitch_down(self):
        '''
        :description: whether motion status is pitch down
        
        :example:

        .. code-block:: python
            :linenos:

            while True:
                print("is_pitch_down:",board.is_pitch_down)
                sleep(0.1)
        '''
        if self._motion is None:
            self._motion = Motion(self)
            return 0
        return self._motion.pitch>15
    
    def is_roll_left(self):
        '''
        :description: whether motion status is roll left
        
        :example:

        .. code-block:: python
            :linenos:

            while True:
                print("is_roll_left:",board.is_roll_left)
                sleep(0.1)
        '''
        if self._motion is None:
            self._motion = Motion(self)
            return 0
        return self._motion.roll<-15

    def is_roll_right(self):
        '''
        :description: whether motion status is roll right
        
        :example:

        .. code-block:: python
            :linenos:

            while True:
                print("is_roll_right:",board.is_roll_right)
                sleep(0.1)
        '''
        if self._motion is None:
            self._motion = Motion(self)
            return 0
        return self._motion.roll>15

    @property
    def mbuild(self):
        if self._dev is None:
            self.set_leds(0,0,0)
        if self._mbuild is None:
            self._mbuild = mBuild(self._dev)
            # attrs = [attr for attr in dir(self._mbuild) if not attr.startswith('_')]
            # for i in range(len(attrs)):
            #     attr = getattr(self._mbuild,attrs[i])
            #     t = type(attr).__name__.lower()
            #     if t!='method' and t!='protocol' and len(t)>3:
            #         print('    @property')
            #         print('    def',t+'(self):')
            #         print('        return self.mbuild.'+t+'\n')
        return self._mbuild

    @property
    def angle(self):
        return self.mbuild.angle

    @property
    def button(self):
        return self.mbuild.button

    @property
    def color(self):
        return self.mbuild.color

    @property
    def dcmotor(self):
        return self.mbuild.dcmotor

    @property
    def encodermotor(self):
        return self.mbuild.encodermotor

    @property
    def extdcmotor(self):
        return self.mbuild.extdcmotor

    @property
    def flame(self):
        return self.mbuild.flame

    @property
    def gpio(self):
        return self.mbuild.gpio

    @property
    def infrarer(self):
        return self.mbuild.infrarer

    @property
    def joystick(self):
        return self.mbuild.joystick

    @property
    def laserranging(self):
        return self.mbuild.laserranging

    @property
    def ledmatrix(self):
        return self.mbuild.ledmatrix

    @property
    def light(self):
        return self.mbuild.light

    @property
    def magnetic(self):
        return self.mbuild.magnetic

    @property
    def motion(self):
        return self.mbuild.motion

    @property
    def fingertippiano(self):
        return self.mbuild.fingertippiano

    @property
    def pirmotion(self):
        return self.mbuild.pirmotion

    @property
    def powermanager(self):
        return self.mbuild.powermanager

    @property
    def neuronprotocol(self):
        return self.mbuild.neuronprotocol

    @property
    def rgbled(self):
        return self.mbuild.rgbled

    @property
    def servo(self):
        return self.mbuild.servo

    @property
    def slider(self):
        return self.mbuild.slider

    @property
    def smartservo(self):
        return self.mbuild.smartservo

    @property
    def soilmoisture(self):
        return self.mbuild.soilmoisture

    @property
    def sound(self):
        return self.mbuild.sound

    @property
    def speaker(self):
        return self.mbuild.speaker

    @property
    def touch(self):
        return self.mbuild.touch

    @property
    def ultrasonic(self):
        return self.mbuild.ultrasonic
