# -*- coding: utf-8 -*
from makeblock.utils import *
from makeblock.protocols.PackData import MegaPiPackData
from time import sleep,time
class _BaseModule:
    def __init__(self,board,port=0,slot=0,type=0):
        self.setup(board,port,slot,type)

    def setup(self,board,port=0,slot=0,type=0):
        board._autoconnect()
        self._callback = None
        self._is_received = False
        self._board = board
        self._type = type
        self._pack = MegaPiPackData()
        self._pack.port = port
        self._pack.slot = slot
        self._last_time = 0
        self._init_module()

    def _init_module(self):
        pass

    def request(self,pack):
        self._board.remove_response(pack)
        self._board.request(pack)

    def call(self,pack):
        self._board.call(pack)

    def _run_callback(self,value):
        if not self._callback is None:
            self._callback(value)

    def _on_parse(self, pack):
        self._pack.data = pack.data
        self._is_received = True

    def read(self,data,callback=None):
        if time()-self._last_time>0.01:
            self._last_time = time()
        else:
            return []
        if not callback is None:
            self._callback = callback
        self._pack.data = data
        self._is_received = False
        self.request(self._pack)
        if callback is None:
            timeout = 100
            while not self._is_received:
                timeout-=1
                sleep(0.001)
                if timeout<0:
                    break
            return self._pack.data

class Servo(_BaseModule):
    """
        :description: Servo Driver - |servo_more_info|

        .. |servo_more_info| raw:: html
        
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/adapters/me-rj25-adapter.html" target="_blank">Me RJ25 Adapter</a> | 
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/motors/meds15-servo-motor.html" target="_blank">MEDS15 Servo</a> | 
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/motors/9g-micro-servo.html" target="_blank">9g Micro Servo</a> | 
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/motors/mg995-standard-servo.html" target="_blank">MG995 Servo</a>

        :param board: main controller
        :type board: *
        :param port: Port Number, range：PORT5～PORT8
        :type port: int
        :param slot: Slot Number, range：SLOT1～SLOT2
        :type slot: int

        :example:
            
        .. code-block:: python
            :linenos:

            servo = Servo(board,PORT6,SLOT1)

    """
    def _init_module(self):
        self._pack.action = MegaPiPackData.ACTION_RUN
        self._pack.module = 0x0B
    
    def set_angle(self,angle,port=0,slot=0):
        """
            set_angle

            :param angle: angle (°), range: 0~180
            :type angle: int

            :example:
                
            .. code-block:: python
                :linenos:

                while True:
                    servo.set_angle(30)
                    sleep(1)
                    servo.set_angle(120)
                    sleep(1)
                
        """
        port = port or self._pack.port
        slot = slot or self._pack.slot
        self._pack.data = [port, slot ,angle]
        self.call(self._pack)

class DCMotor(_BaseModule):
    """
        :description: DC Motor Driver - |dc_motor_more_info|

        .. |dc_motor_more_info| raw:: html
        
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/motor-drivers/megapi-encoder-dc-driver-v1.html" target="_blank">MegaPi Encoder/DC Motor Driver</a> | 
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/motors/36-dc-geared-motor-12v240rpm.html" target="_blank">36 DC Geared Motor 12V 240RPM</a> |
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/motors/dc-motor-25-6v.html" target="_blank">DC Motor-25 6V</a> |
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/motors/dc-motor-37-12v.html" target="_blank">DC Motor-37 12V</a> |
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/motors/mini-metal-gear-motor-n20-dc-12v.html" target="_blank">Mini Metal Gear Motor – N20 DC 12V</a> |
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/motors/tt-geared-motor-dc-6v-200rpm.html" target="_blank">TT Geared Motor DC 6V-200RPM</a> | 
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/motors/air-pump-motor-dc-12v-3202pm.html" target="_blank">Air Pump Motor DC 12V-3202PM</a> | 
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/motors/micro-peristaltic-pump-dc12-0v.html" target="_blank">Micro Peristaltic Pump DC12.0V</a> |
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/motors/air-pump-motor-dc-12v-370-02pm.html" target="_blank">Air Pump Motor – DC 12V-370-02PM</a> | 
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/motors/water-pump-motor-dc-12v-370-04pm.html" target="_blank">Water Pump Motor – DC 12V-370-04PM</a>
            
        :param board: main controller
        :type board: *
        :param port: Port Number, range: PORT1～PORT4, M9~M12
        :type port: int
        :param slot: Slot Number, range: SLOT1～SLOT2
        :type slot: int

        :example:

        .. code-block:: python
            :linenos:

            dcmotor = DCMotor(board,PORT1,SLOT1)

    """
    def _init_module(self):
        self._pack.action = MegaPiPackData.ACTION_RUN

    def run(self,speed,port=0,slot=0):
        """
            :description: motor run with speed

            :param speed: speed (percent), range: -100~100
            :type speed: int
            
            :example:

            .. code-block:: python
                :linenos:

                while True:
                    dcmotor.run(40)
                    sleep(5)
                    dcmotor.run(0)
                    sleep(5)
                    dcmotor.run(-40)
                    sleep(5)

        """
        port = port or self._pack.port
        slot = slot or self._pack.slot
        if port<9:
            self._pack.module = 0x0A
            self._pack.data = [port + (slot - 1) * 8]
        else:
            self._pack.module = 0x41
            self._pack.data = [port]
        self._pack.data.extend(short2bytes(int(speed*2.55)))
        self.call(self._pack)

class StepperMotor(_BaseModule):
    """
        :description: Stepper Motor Driver - |stepper_more_info|

        .. |stepper_more_info| raw:: html
        
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/motor-drivers/megapi-stepper-motor-driver.html" target="_blank">More Info</a> | 
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/motors/42byg-stepper-motor.html" target="_blank">42BYG Stepper Motor</a> | 
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/motors/42byg-geared-stepper-motor.html" target="_blank">42BYG Geared Stepper Motor</a> | 
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/motors/57byg-stepper-motor.html" target="_blank">57BYG Stepper Motor</a>

        :param board: main controller
        :type board: *
        :param slot: Slot Number, range: SLOT1～SLOT4
        :type slot: int

        :example:

        .. code-block:: python
            :linenos:

            stepper = StepperMotor(board,SLOT1)

    """
    def _init_module(self):
        self._pack.action = MegaPiPackData.ACTION_RUN
        self._pack.module = 0x4C
        self._pack.on_response = self._on_parse

    def _on_parse(self,pack):
        self._callback(pack.data[0])

    def move_to(self,position,speed,callback,port=0):
        """
            :description: move to position with speed

            :param position: absolute position ( steps )
            :type position: int
            :param speed: stepper motor speed
            :type speed: int
            :param callback: trig function when moving finish
            :type callback: function
            
            :example:

            .. code-block:: python
                :linenos:

                position = 0
                def on_finished(value):
                    global position
                    position = 5000 - position
                    stepper.move_to(position,10000,on_finished)
                on_finished(position)
        """
        self._callback = callback
        port = port or self._pack.port
        self._pack.data = [0x6,port]
        self._pack.data.extend(long2bytes(position))
        self._pack.data.extend(short2bytes(speed))
        super().request(self._pack)

    def run(self,speed,port=0):
        """
            :description: run with speed

            :param speed: stepper motor speed
            :type speed: int
            
        """
        port = port or self._pack.port
        self._pack.data = [0x2,port]
        self._pack.data.extend(short2bytes(speed))
        super().call(self._pack)

    def set_home(self,port=0):
        """
            :description: set position to zero
            
        """
        port = port or self._pack.port
        self._pack.data = [0x4,port]
        super().call(self._pack)

class EncoderMotor(_BaseModule):
    """
        :description: Encoder Motor Driver - |encoder_more_info|

        .. |encoder_more_info| raw:: html
        
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/motor-drivers/megapi-encoder-dc-driver-v1.html" target="_blank">More Info</a> | 
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/motors/dc-encoder-motor-25-6v-185rpm.html" target="_blank">DC Encoder Motor – 25 6V-185RPM</a>

        :param board: main controller
        :type board: *
        :param slot: Slot Number, range: SLOT1～SLOT4
        :type slot: int

        :example:

        .. code-block:: python
            :linenos:

            encoder = EncoderMotor(board,SLOT1)

    """
    def _init_module(self):
        self._pack.action = MegaPiPackData.ACTION_RUN
        self._pack.module = 0x3E
        self._pack.on_response = self._on_parse

    def _on_parse(self,pack):
        self._callback(pack.data[0])

    def move_to(self,position,speed,callback,port=0):
        """
            :description: move to position with speed

            :param position: absolute position ( steps )
            :type position: int
            :param speed: encoder motor speed
            :type speed: int
            :param callback: trig function when moving finish
            :type callback: function

            :example:

            .. code-block:: python
                :linenos:

                position = 0
                def on_finished(value):
                    position = 5000 - position
                    encoder.move_to(position,100,on_finished)
                on_finished(position)
            
        """
        self._callback = callback
        port = port or self._pack.port
        self._pack.data = [0x6,port]
        self._pack.data.extend(long2bytes(position))
        self._pack.data.extend(short2bytes(speed))
        super().request(self._pack)

    def run(self,speed,port=0):
        """
            :description: run with speed

            :param speed: encoder motor speed
            :type speed: int
            
        """
        port = port or self._pack.port
        self._pack.data = [0x2,port]
        self._pack.data.extend(short2bytes(speed))
        super().call(self._pack)

    def set_home(self,port):
        """
            :description: set position to zero
            
        """
        port = port or self._pack.port
        self._pack.data = [0x4,port]
        super().call(self._pack)

class SmartServo(_BaseModule):
    """
        :description: Smart Servo Driver

        :param board: main controller
        :type board: *

        :example:

        .. code-block:: python
            :linenos:

            smartservo = SmartServo(board)

    """
    def _init_module(self):
        
        self._pack.action = MegaPiPackData.ACTION_RUN
        self._pack.module = 0x40

    def set_led(self,idx,red,green,blue,port=0):
        port = port or self._pack.port
        self._pack.data = [2,port,idx,red,green,blue]
        self.call(self._pack)

    def move_to(self,idx,position,speed,port=0):
        port = port or self._pack.port
        self._pack.data = [4,port,idx]
        self._pack.data.extend(long2bytes(position))
        self._pack.data.extend(float2bytes(speed))
        self.call(self._pack)

    def move(self,idx,position,speed,port=0):
        port = port or self._pack.port
        self._pack.data = [5,port,idx]
        self._pack.data.extend(long2bytes(position))
        self._pack.data.extend(float2bytes(speed))
        self.call(self._pack)

    def set_zero(self,idx,port=0):
        port = port or self._pack.port
        self._pack.data = [7,port,idx]
        self.call(self._pack)

    def run(self,idx,pwm,port=0):
        port = port or self._pack.port
        self._pack.data = [6,port,idx]
        self._pack.data.extend(short2bytes(pwm))
        self.call(self._pack)

    def set_break(self,idx,status,port=0):
        port = port or self._pack.port
        self._pack.data = [1,port,idx,status]
        self.call(self._pack)

    def set_init(self,idx,port):
        port = port or self._pack.port
        self._pack.data = [8,port,idx]
        self.call(self._pack)


class BLDCMotor(_BaseModule):
    def _init_module(self):
        self._pack.action = MegaPiPackData.ACTION_RUN
        self._pack.module = 0x44

    def run(self,pwm,port=0):
        port = port or self._pack.port
        self._pack.data = [2,port,pwm]
        self.call(self._pack)

class Buzzer(_BaseModule):
    def _init_module(self):
        self._pack.action = MegaPiPackData.ACTION_RUN
        self._pack.module = 0x22

    def set_tone(self,hz,ms=0,port=0):
        port = port or self._pack.port
        self._pack.data = [port]
        self._pack.data.extend(short2bytes(hz))
        self._pack.data.extend(short2bytes(ms))
        self.call(self._pack)

class RGBLed(_BaseModule):
    """
        :description: RGB Led Driver - |rgbled_more_info|

        .. |rgbled_more_info| raw:: html
        
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/displays/me-rgb-led.html" target="_blank">Me RGB LED</a> | 
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/adapters/me-rj25-adapter.html" target="_blank">Me RJ25 Adapter</a> | 
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/displays/led-rgb-strip-addressable-sealed-0-5m1m.html" target="_blank">LED RGB Strip</a> | 

        :param board: main controller
        :type board: *
        :param port: Port Number
        :type port: int
        :param slot: Slot Number when using led rgb strip, range: SLOT1～SLOT2
        :type slot: int

        :example:

        .. code-block:: python
            :linenos:

            rgbled = RGBLed(board,PORT6)

    """
    def _init_module(self):
        self._pack.action = MegaPiPackData.ACTION_RUN
        self._pack.module = 0x08

    def set_color(self,index,red,green,blue,port=0,slot=0):
        """
            :description: set color for led

            :param index: led index, 0 for all, range: >=0
            :type index: int
            :param red: color red, range: 0~255
            :type red: int
            :param blue: color blue, range: 0~255
            :type blue: int
            :param green: color green, range: 0~255
            :type green: int
            
            :example:

            .. code-block:: python
                :linenos:

                while True:
                    rgbled.set_pixel(0,0xff,0x0,0x0)
                    sleep(1)
                    rgbled.set_pixel(0,0x0,0xff,0x0)
                    sleep(1)
                    rgbled.set_pixel(0,0x0,0x0,0xff)
                    sleep(1)
        """
        port = port or self._pack.port
        slot = slot or self._pack.slot
        self._pack.data = [port,slot,index,red,green,blue]
        self.call(self._pack)

    def set_colors(self,pixels,port=0,slot=0):
        """
            :description: set colors for all leds

            :param pixels: rgb colors for all leds, [red1,green1,blue1,red2,green2,blue2...]
            :type pixels: list
        """
        port = port or self._pack.port
        slot = slot or self._pack.slot
        if port>61:
            if port==62:
                port = 6
                slot = 1
            if port==63:
                port = 6
                slot = 2
            if port==64:
                port = 7
                slot = 1
            if port==65:
                port = 7
                slot = 2
            if port==66:
                port = 8
                slot = 1
            if port==67:
                port = 8
                slot = 2
        self._pack.data = [port,slot,0]
        self._pack.data.extend(pixels)
        self.call(self._pack)

class SevenSegmentDisplay(_BaseModule):
    """
        :description: Seven Segment Display - |7segdisplay_more_info|

        .. |7segdisplay_more_info| raw:: html
        
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/displays/me-7-segment-display.html" target="_blank">More Info</a>

        :param board: main controller
        :type board: *
        :param port: Port Number, range: PORT5~PORT8
        :type port: int

        :example:

        .. code-block:: python
            :linenos:

            sevseg = SevenSegmentDisplay(board,PORT6)

    """
    def _init_module(self):
        self._pack.action = MegaPiPackData.ACTION_RUN
        self._pack.module = 0x9

    def set_number(self,number,port=0):
        """
            :description: display number

            :param number: number
            :type number: float
            
            :example:

            .. code-block:: python
                :linenos:

                i = 0.0
                while True:
                    sevseg.set_number(i)
                    i+=0.4
                    if i>10.0:
                        i=0.0
                    sleep(1)

        """
        port = port or self._pack.port
        self._pack.data = [port]
        self._pack.data.extend(float2bytes(number))
        self.call(self._pack)

class LedMatrix(_BaseModule):
    """
        :description: LED Matrix Display - |ledmatrix_more_info|

        .. |ledmatrix_more_info| raw:: html
        
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/displays/me-led-matrix-8x16.html" target="_blank">More Info</a>

        :param board: main controller
        :type board: *
        :param port: Port Number, range: PORT5~PORT8
        :type port: int

        :example:

        .. code-block:: python
            :linenos:

            ledmatrix = LedMatrix(board,PORT6)

    """
    def _init_module(self):
        self._pack.action = MegaPiPackData.ACTION_RUN
        self._pack.module = 0x29

    def set_string(self,msg,x=0,y=0,port=0):
        """
            :description: display string

            :param msg: show message
            :type msg: str

            :example:

            .. code-block:: python
                :linenos:

                while True:
                    sleep(1)
                    ledmatrix.set_string('hello')
                    sleep(1)
                    ledmatrix.set_string('world')

        """
        port = port or self._pack.port
        self._pack.data = [port,1,int2uint(x),int2uint(8-y),len(msg)]
        self._pack.data.extend(string2bytes(msg))
        self.call(self._pack)

    def set_pixels(self,pixels,port=0):
        """
            :description: show leds by pixels

            :param pixels: 2 bytes
            :type pixels: list

            :example:

            .. code-block:: python
                :linenos:

                while True:
                    sleep(1)
                    ledmatrix.set_pixels([0xff,0xff])
                    sleep(1)
                    ledmatrix.set_pixels([0x0,0x0])

        """
        port = port or self._pack.port
        self._pack.data = [port,2,0,0]
        self._pack.data.extend(pixels)
        self.call(self._pack)

    def set_time(self,hours,minutes,colon=1,port=0):
        """
            :description: show time

            :param hours: hours
            :type hours: int
            :param minutes: minutes
            :type minutes: int
            :param colon: show colon
            :type colon: bool

            :example:

            .. code-block:: python
                :linenos:

                while True:
                    sleep(1)
                    ledmatrix.set_pixels([0xff,0xff])
                    sleep(1)
                    ledmatrix.set_pixels([0x0,0x0])

        """
        port = port or self._pack.port
        self._pack.data = [port,3,colon,hours,minutes]
        self.call(self._pack)

    def set_number(self,number,port=0):
        """
            :description: show number

            :param number: number
            :type number: float

            :example:

            .. code-block:: python
                :linenos:

                i=0.0
                while True:
                    i+=0.4
                    if i>10:
                        i=0.0
                    ledmatrix.set_number(i)
                    sleep(1)

        """
        port = port or self._pack.port
        self._pack.data = [port,4]
        self._pack.data.extend(float2bytes(number))
        self.call(self._pack)

class DSLRShutter(_BaseModule):
    """
        :description: Shutter for DSLR - |shutter_more_info|

        .. |shutter_more_info| raw:: html
        
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/execution/me-shutter.html" target="_blank">More Info</a>

        :param board: main controller
        :type board: *
        :param port: Port Number, range: PORT5~PORT8
        :type port: int

        :example:

        .. code-block:: python
            :linenos:

            shutter = DSLRShutter(board,PORT6)

    """
    def _init_module(self):
        self._pack.action = MegaPiPackData.ACTION_RUN
        self._pack.module = 0x14

    def turn_on(self,port=0):
        """
            :description: turn on

            :example:

            .. code-block:: python
                :linenos:

                while True:
                    sleep(5)
                    shutter.turn_on()
                    sleep(0.1)
                    shutter.turn_off()

        """
        port = port or self._pack.port
        self._pack.data = [port,1]
        self.call(self._pack)

    def turn_off(self,port=0):
        """
            :description: turn off

        """
        port = port or self._pack.port
        self._pack.data = [port,2]
        self.call(self._pack)

class InfraredReceiver(_BaseModule):
    """
        :description: Infrared Receiver - |infrared_more_info|

        .. |infrared_more_info| raw:: html
        
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/communicators/me-infrared-reciver-decode.html" target="_blank">More Info</a>

        :param board: main controller
        :type board: *
        :param port: Port Number, range: PORT5~PORT8
        :type port: int

        :example:

        .. code-block:: python
            :linenos:

            ir = InfraredReceiver(board,PORT6)

    """
    def _init_module(self):
        self._pack.action = MegaPiPackData.ACTION_GET
        self._pack.module = 0x10
        self._pack.on_response = self._on_parse

    def _on_parse(self, pack):
        super()._on_parse(pack)
        super()._run_callback(bytes2float(pack.data,1))

    def read(self,callback,port=0):
        """
            :description: read ir code

            :param callback: trig when ir code has been received 
            :type callback: function

            :example:

            .. code-block:: python
                :linenos:

                def onReceived(value):
                    print("value:",value)

                while True:
                    ir.read(onReceived)
                    sleep(1)

        """
        port = port or self._pack.port
        res = super().read([port],callback)
        if len(res)>3:
            return bytes2float(res,1)
        return 0

class Ultrasonic(_BaseModule):
    """
        :description: Ultrasonic Sensor - |ultrasonic_more_info|

        .. |ultrasonic_more_info| raw:: html
        
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/sensors/me-ultrasonic-sensor.html" target="_blank">More Info</a>

        :param board: main controller
        :type board: *
        :param port: Port Number, range: PORT5~PORT8
        :type port: int

        :example:

        .. code-block:: python
            :linenos:

            us = Ultrasonic(board,PORT6)

    """
    def _init_module(self):
        self._pack.action = MegaPiPackData.ACTION_GET
        self._pack.module = 0x01
        self._pack.on_response = self._on_parse

    def _on_parse(self, pack):
        super()._on_parse(pack)
        super()._run_callback(float('{0:.1f}'.format(bytes2float(pack.data,1))))

    def read(self,callback,port=0):
        """
            :description: read distance asynchronously

            :param callback: trig when distance has been received 
            :type callback: function

            :example:

            .. code-block:: python
                :linenos:

                def onReceived(value):
                    print("distance:",value)

                while True:
                    us.read(onReceived)
                    sleep(1)

        """
        port = port or self._pack.port
        res = super().read([port],callback)
        if len(res)>4:
            return float('{0:.1f}'.format(bytes2float(res,1)))
        return 0.0

    def get_distance(self,port):
        return self.read(None,port)

class Button(_BaseModule):
    """
        :description: Button - |button_more_info|

        .. |button_more_info| raw:: html
        
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/control/me-4-button.html" target="_blank">More Info</a>

        :param board: main controller
        :type board: *
        :param port: Port Number, range: PORT5~PORT8
        :type port: int

        :example:

        .. code-block:: python
            :linenos:

            button = Button(board,PORT6)

    """
    def _init_module(self):
        self._pack.action = MegaPiPackData.ACTION_GET
        self._pack.module = 0x16
        self._pack.on_response = self._on_parse

    def _on_parse(self, pack):
        super()._on_parse(pack)
        super()._run_callback(pack.data[1])

    def read(self,callback,port=0):
        """
            :description: read pressed key code asynchronously

            :param callback: trig when pressed key code has been received 
            :type callback: function

            :example:

            .. code-block:: python
                :linenos:

                def onReceived(value):
                    print("button key pressed:",value)

                while True:
                    button.read(onReceived)
                    sleep(1)

        """
        port = port or self._pack.port
        res = super().read([port,0],callback)
        if len(res)>1:
            return res[1]
        return 0

class ButtonOnBoard(_BaseModule):
    """
        :description: Button - |button_more_info|

        .. |button_more_info| raw:: html
        
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/control/me-4-button.html" target="_blank">More Info</a>

        :param board: main controller
        :type board: *
        :param port: Port Number, range: PORT5~PORT8
        :type port: int

        :example:

        .. code-block:: python
            :linenos:

            button = Button(board,PORT6)

    """
    def _init_module(self):
        self._pack.action = MegaPiPackData.ACTION_GET
        self._pack.module = 0x23
        self._pack.on_response = self._on_parse

    def _on_parse(self, pack):
        super()._on_parse(pack)
        if len(pack.data)>0:
            super()._run_callback(pack.data[1]==1)

    def read(self,callback=None):
        """
            :description: read pressed key code asynchronously

            :param callback: trig when pressed key code has been received 
            :type callback: function

            :example:

            .. code-block:: python
                :linenos:

                def onReceived(value):
                    print("button key pressed:",value)

                while True:
                    button.read(onReceived)
                    sleep(1)

        """
        super().read([7,0],callback)
        if len(self._pack.data)>1:
            return self._pack.data[1]==1
        return False

    def is_pressed(self):
        return self.read()
        
class LineFollower(_BaseModule):
    """
        :description: LineFollower - |linefollower_more_info|

        .. |linefollower_more_info| raw:: html
        
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/sensors/me-line-follower.html" target="_blank">More Info</a>

        :param board: main controller
        :type board: *
        :param port: Port Number, range: PORT5~PORT8
        :type port: int

        :example:

        .. code-block:: python
            :linenos:

            linefollower = LineFollower(board,PORT6)

    """
    def _init_module(self):
        self._pack.action = MegaPiPackData.ACTION_GET
        self._pack.module = 0x11
        self._pack.on_response = self._on_parse

    def _on_parse(self, pack):
        super()._on_parse(pack)
        super()._run_callback(bytes2float(pack.data,1))

    def read(self,callback,port=0):
        """
            :description: read linefollower status asynchronously

            :param callback: trig when linefollower status has been received 
            :type callback: function

            :example:

            .. code-block:: python
                :linenos:

                def onReceived(value):
                    print("linefollower status:",value)

                while True:
                    linefollower.read(onReceived)
                    sleep(1)

        """
        port = port or self._pack.port
        res = super().read([port],callback)
        if len(res)>1:
            return res[1]
        return -1

    def get_status(self,port=0):
        return self.read(None,port)

class LimitSwitch(_BaseModule):
    """
        :description: LimitSwitch - |limitswitch_more_info|

        .. |limitswitch_more_info| raw:: html
        
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/adapters/me-rj25-adapter.html" target="_blank">Me RJ25 Adapter</a> | 
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/sensors/me-micro-switch-ab.html" target="_blank">Me Micro Switch A</a>

        :param board: main controller
        :type board: *
        :param port: Port Number, range: PORT5~PORT8
        :type port: int
        :param slot: Slot Number, range: SLOT1~SLOT2
        :type slot: int

        :example:

        .. code-block:: python
            :linenos:

            limitswitch = LimitSwitch(board,PORT6,SLOT1)

    """
    def _init_module(self):
        self._pack.action = MegaPiPackData.ACTION_GET
        self._pack.module = 0x15
        self._pack.on_response = self._on_parse

    def _on_parse(self, pack):
        super()._on_parse()
        super()._run_callback(bytes2float(pack.data,1))

    def read(self,callback,port=0,slot=0):
        """
            :description: read limitswitch status asynchronously

            :param callback: trig when limitswitch status has been received 
            :type callback: function

            :example:

            .. code-block:: python
                :linenos:

                def onReceived(value):
                    print("limitswitch status:",value)

                while True:
                    limitswitch.read(onReceived)
                    sleep(1)

        """
        port = port or self._pack.port
        slot = slot or self._pack.slot
        res = super().read([port,slot],callback)
        if len(res)>1:
            return res[1]
        return False
    
    def get_status(self,port=0,slot=0):
        return self.read(None,port,slot)

class PIRMotion(_BaseModule):
    """
        :description: PIR Motion - |pirmotion_more_info|

        .. |pirmotion_more_info| raw:: html
        
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/sensors/me-pir-motion-sensor.html" target="_blank">More Info</a>

        :param board: main controller
        :type board: *
        :param port: Port Number, range: PORT5~PORT8
        :type port: int

        :example:

        .. code-block:: python
            :linenos:

            pir = PIRMotion(board,PORT6)

    """
    def _init_module(self):
        self._pack.action = MegaPiPackData.ACTION_GET
        self._pack.module = 0x0F
        self._pack.on_response = self._on_parse

    def _on_parse(self, pack):
        super()._on_parse(pack)
        super()._run_callback(bytes2float(pack.data,1))

    def read(self,callback,port=0):
        """
            :description: read pirmotion status asynchronously

            :param callback: trig when pirmotion status has been received 
            :type callback: function

            :example:

            .. code-block:: python
                :linenos:

                def onReceived(value):
                    print("pirmotion status:",value)

                while True:
                    pir.read(onReceived)
                    sleep(1)

        """
        port = port or self._pack.port
        res = super().read([port],callback)
        if len(res)>1:
            return res[1]
        return 0
    
    def get_status(self,port=0):
        return self.read(None,port)

class Light(_BaseModule):
    """
        :description: Light Sensor - |light_more_info|

        .. |light_more_info| raw:: html
        
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/sensors/me-light-sensor.html" target="_blank">More Info</a>

        :param board: main controller
        :type board: *
        :param port: Port Number, range: PORT5~PORT8
        :type port: int

        :example:

        .. code-block:: python
            :linenos:

            light = Light(board,PORT6)

    """
    def _init_module(self):
        self._pack.action = MegaPiPackData.ACTION_GET
        self._pack.module = 0x03
        self._pack.on_response = self._on_parse

    def _on_parse(self, pack):
        super()._on_parse(pack)
        super()._run_callback(bytes2float(pack.data,1))

    def read(self,callback,port=0):
        """
            :description: read brightness asynchronously

            :param callback: trig when brightness has been received 
            :type callback: function

            :example:

            .. code-block:: python
                :linenos:

                def onReceived(value):
                    print("brightness:",value)

                while True:
                    light.read(onReceived)
                    sleep(1)

        """
        port = port or self._pack.port
        res = super().read([port,2],callback)
        if len(res)>4:
            return int(bytes2float(res,1))
        return 0

    def get_lightness(self,port):
        return self.read(None,port)

class Sound(_BaseModule):
    """
        :description: Sound Sensor - |sound_more_info|

        .. |sound_more_info| raw:: html
        
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/sensors/me-sound-sensor.html" target="_blank">More Info</a>

        :param board: main controller
        :type board: *
        :param port: Port Number, range: PORT5~PORT8
        :type port: int

        :example:

        .. code-block:: python
            :linenos:

            sound = Sound(board,PORT6)

    """
    def _init_module(self):
        self._pack.action = MegaPiPackData.ACTION_GET
        self._pack.module = 0x07
        self._pack.on_response = self._on_parse

    def _on_parse(self, pack):
        super()._on_parse(pack)
        super()._run_callback(bytes2float(pack.data,1))

    def read(self,callback,port=0):
        """
            :description: read loudness asynchronously

            :param callback: trig when loudness has been received 
            :type callback: function

            :example:

            .. code-block:: python
                :linenos:

                def onReceived(value):
                    print("loudness:",value)

                while True:
                    sound.read(onReceived)
                    sleep(1)

        """
        port = port or self._pack.port
        res = super().read([port],callback)
        if len(res)>4:
            return bytes2float(res,1)
        return 0.0

class Potentiometer(_BaseModule):
    """
        :description: Potentiometer - |potentiometer_more_info|

        .. |potentiometer_more_info| raw:: html
        
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/control/me-potentiometer.html" target="_blank">More Info</a>

        :param board: main controller
        :type board: *
        :param port: Port Number, range: PORT5~PORT8
        :type port: int

        :example:

        .. code-block:: python
            :linenos:

            potentiometer = Potentiometer(board,PORT6)

    """
    def _init_module(self):
        self._pack.action = MegaPiPackData.ACTION_GET
        self._pack.module = 0x04
        self._pack.on_response = self._on_parse

    def _on_parse(self, pack):
        super()._on_parse(pack)
        super()._run_callback(bytes2float(pack.data,1))

    def read(self,callback,port=0):
        """
            :description: read potentiometer asynchronously

            :param callback: trig when potentiometer has been received 
            :type callback: function

            :example:

            .. code-block:: python
                :linenos:

                def onReceived(value):
                    print("potentiometer:",value)

                while True:
                    potentiometer.read(onReceived)
                    sleep(1)

        """
        port = port or self._pack.port
        res = super().read([port],callback)
        if len(res)>4:
            return bytes2float(res,1)
        return 0.0

class Joystick(_BaseModule):
    """
        :description: Joystick - |joystick_more_info|

        .. |joystick_more_info| raw:: html
        
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/control/me-joystick.html" target="_blank">More Info</a>

        :param board: main controller
        :type board: *
        :param port: Port Number, range: PORT5~PORT8
        :type port: int

        :example:

        .. code-block:: python
            :linenos:

            joystick = Joystick(board,PORT6)

    """
    def _init_module(self):
        self._pack.action = MegaPiPackData.ACTION_GET
        self._pack.module = 0x05
        self._pack.on_response = self._on_parse
        self._pos = {
            'x':0,
            'y':0
        }

    def _on_parse(self, pack):
        super()._on_parse(pack)
        super()._run_callback(bytes2short(pack.data,1))

    def run(self,left,right):
        pack = MegaPiPackData()
        pack.action = MegaPiPackData.ACTION_RUN
        pack.module = 0x05
        pack.data = short2bytes(left).extend(short2bytes(right))
        super().call(pack)

    def read(self,callback,port=0,slot=0):
        """
            :description: read joystick data asynchronously

            :param callback: trig when joystick data has been received 
            :type callback: function

            :example:

            .. code-block:: python
                :linenos:

                def onReceived(values):
                    print("joystick:",values)

                while True:
                    joystick.read(onReceived)
                    sleep(1)

        """
        port = port or self._pack.port
        slot = slot or self._pack.slot
        res = super().read([port,slot],callback)
        if len(res)>4:
            return bytes2float(res,1)
        return 0.0

    def get_x(self,port=0):
        return self.read(None,port,1)

    def get_y(self,port=0):
        return self.read(None,port,2)
        
class Gyro(_BaseModule):
    """
        :description: Gyro Sensor - |gyro_more_info|

        .. |gyro_more_info| raw:: html
        
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/sensors/me-3-axis-accelerometer-and-gyro-sensor.html" target="_blank">More Info</a>

        :param board: main controller
        :type board: *

        :example:

        .. code-block:: python
            :linenos:

            gyro = Gyro(board)

    """
    def _init_module(self):
        self._pack.action = MegaPiPackData.ACTION_GET
        self._pack.module = 0x06
        self._pack.on_response = self._on_parse

    def _on_parse(self, pack):
        super()._on_parse(pack)
        if len(pack.data)>4:
            super()._run_callback(float('{0:.1f}'.format(bytes2float(pack.data,1))))

    def read(self,axis,callback=None):
        """
            :description: read gyro data asynchronously

            :param callback: trig when gyro data has been received 
            :type callback: function

            :example:

            .. code-block:: python
                :linenos:

                def onReceived(values):
                    print("gyro:",values)

                while True:
                    gyro.read(onReceived)
                    sleep(1)

        """
        res = super().read([self._pack.port,axis],callback)
        if len(res)>4:
            return float('{0:.1f}'.format(bytes2float(res,1)))
        return 0.0

    def get_x(self,callback):
        return self.read(1,callback)

    def get_y(self,callback):
        return self.read(2,callback)
        
    def get_z(self,callback):
        return self.read(3,callback)
        
class Compass(_BaseModule):
    """
        :description: Compass Sensor - |compass_more_info|

        .. |compass_more_info| raw:: html
        
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/sensors/me-compass.html" target="_blank">More Info</a>

        :param board: main controller
        :type board: *
        :param port: Port Number, range: PORT5~PORT8
        :type port: int

        :example:

        .. code-block:: python
            :linenos:

            compass = Compass(board,PORT6)

    """
    def _init_module(self):
        self._pack.action = MegaPiPackData.ACTION_GET
        self._pack.module = 0x1A
        self._pack.on_response = self._on_parse

    def _on_parse(self, pack):
        super()._on_parse(pack)
        super()._run_callback(float('{0:.1f}'.format(bytes2float(pack.data,1))))

    def read(self,callback):
        """
            :description: read compass data asynchronously

            :param callback: trig when compass data has been received 
            :type callback: function

            :example:

            .. code-block:: python
                :linenos:

                def onReceived(value):
                    print("compass:",value)

                while True:
                    compass.read(onReceived)
                    sleep(1)

        """
        res = super().read([],callback)
        if len(res)>4:
            return float('{0:.1f}'.format(bytes2float(res,1)))
        return 0.0

    def get_heading(self):
        return self.read(None)

class Temperature(_BaseModule):
    """
        :description: Temperature - |temperature_more_info|

        .. |temperature_more_info| raw:: html
        
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/adapters/me-rj25-adapter.html" target="_blank">Me RJ25 Adapter</a> | 
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/sensors/temperature-sensor-waterproofds18b20.html" target="_blank">Me Temperature Sensor</a>

        :param board: main controller
        :type board: *
        :param port: Port Number, range: PORT5~PORT8
        :type port: int
        :param slot: Slot Number, range: SLOT1~SLOT2
        :type slot: int

        :example:

        .. code-block:: python
            :linenos:

            temp = Temperature(board,PORT6,SLOT1)

    """
    def _init_module(self):
        self._pack.action = MegaPiPackData.ACTION_GET
        self._pack.module = 0x02
        self._pack.on_response = self._on_parse

    def _on_parse(self, pack):
        super()._on_parse(pack)
        super()._run_callback(float('{0:.1f}'.format(bytes2float(pack.data,1))))

    def read(self,callback,port=0,slot=0):
        """
            :description: read temperature asynchronously

            :param callback: trig when temperature has been received 
            :type callback: function

            :example:

            .. code-block:: python
                :linenos:

                def onReceived(value):
                    print("temperature:",value)

                while True:
                    temp.read(onReceived)
                    sleep(1)

        """
        port = port or self._pack.port
        slot = slot or self._pack.slot
        res = super().read([port,slot],callback)
        if len(res)>4:
            return float('{0:.1f}'.format(bytes2float(res,1)))
        return 0.0

    def get_temperature(self,port=0,slot=0):
        return self.read(None,port,slot)

class TemperatureOnBoard(_BaseModule):
    def _init_module(self):
        self._pack.action = MegaPiPackData.ACTION_GET
        self._pack.module = 0x1b
        self._pack.on_response = self._on_parse

    def _on_parse(self, pack):
        super()._on_parse(pack)
        super()._run_callback(float('{0:.1f}'.format(bytes2float(pack.data,1))))

    def read(self,callback,port=0):
        port = port or self._pack.port
        res = super().read([port],callback)
        if len(res)>4:
            return float('{0:.1f}'.format(bytes2float(res,1)))
        return 0.0

    def get_temperature(self,port=0):
        return self.read(None,port)

class Humiture(_BaseModule):
    """
        :description: Humiture Sensor - |humiture_more_info|

        .. |humiture_more_info| raw:: html
        
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/sensors/me-temperature-and-humidity-sensor.html" target="_blank">More Info</a>

        :param board: main controller
        :type board: *
        :param port: Port Number, range: PORT5~PORT8
        :type port: int

        :example:

        .. code-block:: python
            :linenos:

            humiture = Humiture(board,PORT6)

    """
    def _init_module(self):
        self._pack.action = MegaPiPackData.ACTION_GET
        self._pack.module = 0x17
        self._pack.on_response = self._on_parse

    def _on_parse(self, pack):
        super()._on_parse(pack)
        super()._run_callback(pack.data[1])

    def read(self,type,callback,port=0):
        """
            :description: read humiture and temperature asynchronously

            :param callback: trig when humiture and temperature data has been received 
            :type callback: function

            :example:

            .. code-block:: python
                :linenos:

                def onReceived(hum,temp):
                    print("humiture:",hum," temperature:",temp)

                while True:
                    humiture.read(onReceived)
                    sleep(1)

        """
        port = port or self._pack.port
        res = super().read([port,type],callback)
        if len(res)>1:
            return res[1]
        return 0

    def get_humidity(self,port):
        return self.read(0,None,port)

    def get_temperature(self,port):
        return self.read(1,None,port)

class Flame(_BaseModule):
    """
        :description: Flame Sensor - |flame_more_info|

        .. |flame_more_info| raw:: html
        
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/sensors/me-flame-sensor.html" target="_blank">More Info</a>

        :param board: main controller
        :type board: *
        :param port: Port Number, range: PORT5~PORT8
        :type port: int

        :example:

        .. code-block:: python
            :linenos:

            flame = Flame(board,PORT6)

    """
    def _init_module(self):
        self._pack.action = MegaPiPackData.ACTION_GET
        self._pack.module = 0x18
        self._pack.on_response = self._on_parse

    def _on_parse(self, pack):
        super()._on_parse(pack)
        super()._run_callback(bytes2short(pack.data,1))

    def read(self,callback,port=0):
        """
            :description: read flame status asynchronously

            :param callback: trig when flame status has been received 
            :type callback: function

            :example:

            .. code-block:: python
                :linenos:

                def onReceived(value):
                    print("flame:",value)

                while True:
                    flame.read(onReceived)
                    sleep(1)

        """
        port = port or self._pack.port
        res = super().read([port],callback)
        if len(res)>2:
            return bytes2short(pack.data,1)
        return 0

    def get_level(self,port=0):
        return self.read(None,port)

class Gas(_BaseModule):
    """
        :description: Gas Sensor - |gas_more_info|

        .. |gas_more_info| raw:: html
        
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/sensors/me-gas-sensormq2.html" target="_blank">More Info</a>

        :param board: main controller
        :type board: *
        :param port: Port Number, range: PORT5~PORT8
        :type port: int

        :example:

        .. code-block:: python
            :linenos:

            gas = Gas(board,PORT6)

    """
    def _init_module(self):
        self._pack.action = MegaPiPackData.ACTION_GET
        self._pack.module = 0x19
        self._pack.on_response = self._on_parse

    def _on_parse(self, pack):
        super()._on_parse(pack)
        super()._run_callback(bytes2short(pack.data,1))

    def read(self,callback,port=0):
        """
            :description: read gas status asynchronously

            :param callback: trig when gas status has been received 
            :type callback: function

            :example:

            .. code-block:: python
                :linenos:

                def onReceived(value):
                    print("gas:",value)

                while True:
                    gas.read(onReceived)
                    sleep(1)

        """
        port = port or self._pack.port
        res = super().read([port],callback)
        if len(res)>2:
            return bytes2short(pack.data,1)
        return 0

    def get_level(self,port=0):
        return self.read(None,port)

class Touch(_BaseModule):
    """
        :description: Touch Sensor - |touch_more_info|

        .. |touch_more_info| raw:: html
        
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/sensors/me-touch-sensor.html" target="_blank">More Info</a>

        :param board: main controller
        :type board: *
        :param port: Port Number, range: PORT5~PORT8
        :type port: int

        :example:

        .. code-block:: python
            :linenos:

            touch = Touch(board,PORT6)

    """
    def _init_module(self):
        self._pack.action = MegaPiPackData.ACTION_GET
        self._pack.module = 0x33
        self._pack.on_response = self._on_parse

    def _on_parse(self, pack):
        super()._on_parse(pack)
        super()._run_callback(pack.data[1])

    def read(self,callback,port=0):
        """
            :description: read touch status asynchronously

            :param callback: trig when touch status has been received 
            :type callback: function

            :example:

            .. code-block:: python
                :linenos:

                def onReceived(value):
                    print("touch:",value)

                while True:
                    touch.read(onReceived)
                    sleep(1)

        """
        port = port or self._pack.port
        res = super().read([port],callback)
        if len(res)>1:
            return res.data[1]
        return 0

    def get_status(self,port=0):
        return self.read(None,port)

class Color(_BaseModule):
    def _init_module(self):
        self._pack.action = MegaPiPackData.ACTION_RUN
        self._pack.module = 0x43

    def _on_parse(self, pack):
        super()._on_parse(pack)
        super()._run_callback(pack.data[1])

    def read(self,type,callback,port):
        port = port or self._pack.port
        res = super().read([port,0x1,type],callback)
        if len(res)>1:
            return res.data[1]
        return 0

    def get_red(self,port=0):
        return self.read(0,None,port)

    def get_green(self,port=0):
        return self.read(1,None,port)

    def get_blue(self,port=0):
        return self.read(2,None,port)

class Pin(_BaseModule):
    """
        :description: Pin - |pin_more_info|

        .. |pin_more_info| raw:: html
        
            <a href="http://docs.makeblock.com/diy-platform/en/electronic-modules/main-control-boards/megapi.html" target="_blank">More Info</a>

        :param board: main controller
        :type board: *

        :example:

        .. code-block:: python
            :linenos:

            pin = Pin(board)

    """
    MODE_DIGITAL = 0x1E
    MODE_ANALOG = 0x1F
    MODE_PWM = 0x20
    def _init_module(self):
        self._pack.on_response = self._on_parse

    def _on_parse(self, pack):
        super()._on_parse(pack)
        super()._run_callback(bytes2float(pack.data,1))

    def digital_write(self,pin,level):
        """
            :description: set digital pin output

            :param pin: digital io number 
            :type pin: int
            :param pwm: pwm value, range: 0~1
            :type pwm: int

            :example:

            .. code-block:: python
                :linenos:

                while True:
                    pin.digital_write(5,1)
                    sleep(1)
                    pin.digital_write(5,0)
                    sleep(1)

        """
        pack = MegaPiPackData()
        pack.module = Pin.MODE_DIGITAL
        pack.action = MegaPiPackData.ACTION_RUN
        pack.data = [pin,level]
        self.call(pack)

    def pwm_write(self,pin,pwm):
        """
            :description: set pwm pin output

            :param pin: pwm pin number 
            :type pin: int
            :param pwm: pwm value, range: 0~255
            :type pwm: int

            :example:

            .. code-block:: python
                :linenos:

                while True:
                    pin.pwm_write(5,100)
                    sleep(1)
                    pin.pwm_write(5,0)
                    sleep(1)

        """
        pack = MegaPiPackData()
        pack.module = Pin.MODE_PWM
        pack.action = MegaPiPackData.ACTION_RUN
        pack.data = [pin,pwm]
        self.call(pack)

    def analog_read(self,pin,callback=None):
        """
            :description: read analog pin status asynchronously

            :param callback: trig when analog pin status has been received 
            :type callback: function

            :example:

            .. code-block:: python
                :linenos:

                def onReceived(value):
                    print("pin:",value)

                while True:
                    pin.read(onReceived)
                    sleep(1)

        """
        self._pack.module = Pin.MODE_ANALOG
        self._pack.action = MegaPiPackData.ACTION_GET
        res = super().read([pin],callback)
        if res and len(res)>1:
            return bytes2float(res,1)
        return 0

    def digital_read(self,pin,callback=None):
        """
            :description: read digital pin status asynchronously

            :param callback: trig when digital pin status has been received 
            :type callback: function

            :example:

            .. code-block:: python
                :linenos:

                def onReceived(value):
                    print("pin:",value)

                while True:
                    pin.read(onReceived)
                    sleep(1)

        """
        self._pack.module = Pin.MODE_DIGITAL
        self._pack.action = MegaPiPackData.ACTION_GET
        res = super().read([pin],callback)
        if res and len(res)>1:
            return bytes2float(res,1)
        return 0

class CNC(_BaseModule):
    def _init_module(self):
        pass

    def _on_parse(self,pack):
        pass

    def set_ratio(self,x_ratio,y_ratio,z_ratio):
        pass

    def set_spin(self,power):
        pass

    def moveTo(self,x_position,y_position,z_position):
        pass

    def move(self,x_position,y_position,z_position):
        pass