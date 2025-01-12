# -*- coding: utf-8 -*
import struct
from time import ctime, sleep,time
import threading
import makeblock
import makeblock.utils as utils
from makeblock.protocols.PackData import NeuronPackData

class _BaseModule:
    _pack = None
    _is_received = False
    _last_time = 0
    _callback = None
    def __init__(self,board,idx=1,mode=1,period=0):
        self.setup(board,idx,mode,period)

    def quit(self):
        makeblock.quit()

    def setup(self,board,idx,mode=1,period=0):
        board._autoconnect()
        self._board = board
        self._mode = mode
        self._pack = NeuronPackData()
        self._pack.idx = idx
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

    def _exist_subscribe(self,subscribe_id):
        return self._board._find_subscribe(subscribe_id)

    def subscribe(self,pack=None):
        pass
    _default_data = [0x7f,NeuronPackData.TYPE_CHANGE,0,0,0,0,0]
    def _subscribe(self,idx,service,subservice,on_response,data=None):
        data = data or self._default_data
        subscribe_id = (idx<<24)+(service<<16)+(subservice<<8)
        if self._exist_subscribe(subscribe_id):
            return subscribe_id
        pack = NeuronPackData()
        pack.on_response = on_response
        pack.idx = idx
        pack.service = service
        pack.subservice = subservice
        pack.data = data
        pack.subscribe_id = subscribe_id
        self._board.request(pack)
        return subscribe_id

    def _run_callback(self,value):
        if not self._callback is None:
            self._callback(value)
        return value

    def _on_parse(self, pack):
        self._pack.data = pack.data
        self._is_received = True

    def read(self,idx,service,subservice,data,callback):
        if not callback is None:
            self._callback = callback
        self._is_received = False
        pack = NeuronPackData()
        pack.subscribe_id = idx
        pack.idx = idx
        pack.service = service
        pack.subservice = subservice
        pack.data = data
        pack.on_response = self._pack.on_response
        self.request(pack)
        if callback is None:
            timeout = 100
            while not self._is_received:
                timeout-=1
                sleep(0.001)
                if timeout<0:
                    break
            return self._pack.data

class Temperature(_BaseModule):
    def _init_module(self):
        self._temperature = {}
        self._pack.type = NeuronPackData.TYPE_SENSOR
        self._pack.service = 0x63
        self._pack.subservice = 0x1

    def __on_subscribe_response(self, pack):
        if len(pack.data)>6:
            self._temperature[pack.subscribe_id] = utils.bits2float(pack.data[1:6])
        if self._callback is not None:
            self._callback(self._temperature[pack.subscribe_id])

    def on_change(self,callback):
        self._callback = callback

    def get_temperature(self,idx=None):
        subscribe_id = self.__subscribe(idx)
        if subscribe_id in self._temperature:
            return self._temperature[subscribe_id]
        return 0.0

    def __subscribe(self,idx):
        return super()._subscribe(idx or self._pack.idx,self._pack.service,self._pack.subservice,self.__on_subscribe_response)

class Humiture(_BaseModule):
    def _init_module(self):
        self._pack.service = 0x63
        self._pack.subservice = 0x19
        self._status = {"temp":{},"hum":{}}

    def __on_subscribe_response(self, pack):
        if len(pack.data)>3:
            self._status["temp"][pack.subscribe_id] = utils.bits2short(pack.data[1:3])
            self._status["hum"][pack.subscribe_id] = pack.data[3]
        if self._callback is not None:
            self._callback(self._status["temp"][pack.subscribe_id],self._status["hum"][pack.subscribe_id])

    def on_change(self,callback):
        self._callback = callback

    def get_temperature(self,idx=None):
        subscribe_id = self.__subscribe(idx)
        if subscribe_id in self._status["temp"]:
            return self._status["temp"][subscribe_id]
        return 0
    
    def get_humidity(self,idx=None):
        subscribe_id = self.__subscribe(idx)
        if subscribe_id in self._status["hum"]:
            return self._status["hum"][subscribe_id]
        return 0

    def __subscribe(self,idx=None):
        return super()._subscribe(idx or self._pack.idx,self._pack.service,self._pack.subservice,self.__on_subscribe_response)

class Ultrasonic(_BaseModule):
    def _init_module(self):
        self._distance = 0
        self._pack.service = 0x63
        self._pack.subservice = 0x16
        self._pack.on_response = self._on_parse
        self._distance = {}
        
    def _on_parse(self, pack):
        super()._on_parse(pack)
        self._distance[pack.subscribe_id] = super()._run_callback(float('{0:.1f}'.format(utils.bits2float(pack.data[1:6]))))

    def read(self,idx,callback=None):
        res = super().read(idx,self._pack.service,self._pack.subservice,[0x1],callback)
        if idx in self._distance:
            return self._distance[idx]
        return 0.0

    def get_distance(self,idx=0,callback=None):
        idx = idx or self._pack.idx
        return self.read(idx)

class Slider(_BaseModule):
    def _init_module(self):
        self._value = {}
        self._pack.service = 0x64
        self._pack.subservice = 0xd
        self._pack.on_response = self._on_parse
    
    def _on_parse(self, pack):
        super()._on_parse(pack)
        self._value[pack.subscribe_id] = super()._run_callback(pack.data[1])

    def on_change(self,callback):
        self._callback = callback

    def read(self,idx,callback=None):
        res = super().read(idx,self._pack.service,self._pack.subservice,[0x1],callback)
        if idx in self._value:
            return self._value[idx]
        return 0

    def get_value(self,idx=0,callback=None):
        return self.read(idx or self._pack.idx)

class MQ2(_BaseModule):
    def _init_module(self):
        self._value = {}
        self._pack.service = 0x63
        self._pack.subservice = 0x1c
        self.__subscribe()
    
    def __on_subscribe_response(self,pack):
        if len(pack.data)>1:
            self._value[pack.subscribe_id] = pack.data[1]
    
    def get_value(self,idx=None):
        subscribe_id = self.__subscribe(idx)
        if subscribe_id in self._value:
            return self._value[subscribe_id]
        return 0

    def __subscribe(self,idx=None):
        return super()._subscribe(idx or self._pack.idx,self._pack.service,self._pack.subservice,self.__on_subscribe_response)

class Light(_BaseModule):
    def _init_module(self):
        self._value = {}
        self._pack.service = 0x63
        self._pack.subservice = 0x14
    
    def __on_subscribe_response(self,pack):
        if len(pack.data)>1:
            self._value[pack.subscribe_id] = pack.data[1]
    
    def get_value(self,idx=None):
        subscribe_id = self.__subscribe(idx)
        if subscribe_id in self._value:
            return self._value[subscribe_id]
        return 0

    def __subscribe(self,idx=None):
        return super()._subscribe(self._pack.idx,self._pack.service,self._pack.subservice,self.__on_subscribe_response)
        
class SoilMoisture(_BaseModule):
    def _init_module(self):
        self._value = {}
        self._pack.service = 0x63
        self._pack.subservice = 0x15
    
    def __on_subscribe_response(self,pack):
        if len(pack.data)>1:
            self._value[pack.subscribe_id] = pack.data[1]
    
    def get_value(self,idx=None):
        subscribe_id = self.__subscribe(idx)
        if subscribe_id in self._value:
            return self._value[subscribe_id]
        return 0

    def __subscribe(self,idx=None):
        return super()._subscribe(idx or self._pack.idx,self._pack.service,self._pack.subservice,self.__on_subscribe_response)
     
class Sound(_BaseModule):
    def _init_module(self):
        self._value = {}
        self._pack.service = 0x63
        self._pack.subservice = 0x13
    
    def __on_subscribe_response(self,pack):
        if len(pack.data)>1:
            self._value[pack.subscribe_id] = pack.data[1]
    
    @property
    def get_value(self,idx=None):
        subscribe_id = self.__subscribe(idx)
        if subscribe_id in self._value:
            return self._value[subscribe_id]
        return 0

    def __subscribe(self,idx=None):
        return super()._subscribe(idx or self._pack.idx,self._pack.service,self._pack.subservice,self.__on_subscribe_response)

class Touch(_BaseModule):
    def _init_module(self):
        self._value = {}
        self._pack.service = 0x63
        self._pack.subservice = 0xa
    
    def __on_subscribe_response(self,pack):
        if len(pack.data)>1:
            self._value[pack.subscribe_id] = pack.data[1]
    
    def get_value(self,idx=None):
        subscribe_id = self.__subscribe(idx)
        if subscribe_id in self._value:
            return self._value[subscribe_id]
        return 0

    def is_touched(self,index=0,idx=None):
        return ((self.get_value(idx)>>index)&0x1) == 0x1

    def __subscribe(self,idx=None):
        return super()._subscribe(idx or self._pack.idx,self._pack.service,self._pack.subservice,self.__on_subscribe_response)

class Button(_BaseModule):
    def _init_module(self):
        self._pack.service = 0x64
        self._pack.subservice = 0xb
        self._status = {"is_pressed":{},"count":{}}
    
    def __on_subscribe_response(self,pack):
        if len(pack.data)>1:
            if pack.data[0]==1:
                self._status["is_pressed"][pack.subscribe_id] = pack.data[1]==1
            elif pack.data[0]==2:
                self._status["count"][pack.subscribe_id] = utils.bits2long(pack.data[1:6])
    
    def is_pressed(self,idx=0):
        subscribe_id = self.__subscribe(idx)
        if subscribe_id in self._status["is_pressed"]:
            return self._status["is_pressed"][subscribe_id]
        return False
    
    def get_count(self,idx=0):
        subscribe_id = self.__subscribe(idx)
        if subscribe_id in self._status["count"]:
            return self._status["count"][subscribe_id]
        return 0

    def __subscribe(self,idx=None):
        return super()._subscribe(idx or self._pack.idx,self._pack.service,self._pack.subservice,self.__on_subscribe_response)

    def reset(self,idx=None):
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = self._pack.subservice
        pack.data = [0x3]
        self.call(pack)

class Joystick(_BaseModule):
    def _init_module(self):
        self._pack.service = 0x64
        self._pack.subservice = 0xc
        self._x = {}
        self._y = {}
        self.__subscribe()
    
    def __on_subscribe_response(self,pack):
        if len(pack.data)>1:
            self._x[pack.subscribe_id] = utils.bits2int8(pack.data[1:3])
            self._y[pack.subscribe_id] = utils.bits2int8(pack.data[3:5])
    
    def get_x(self,idx=None):
        subscribe_id = self.__subscribe(idx)
        if subscribe_id in self._x:
            return self._x[subscribe_id]
        return 0
    
    def get_y(self,idx=None):
        subscribe_id = self.__subscribe(idx)
        if subscribe_id in self._y:
            return self._y[subscribe_id]
        return 0

    def __subscribe(self,idx=None):
        return super()._subscribe(idx or self._pack.idx,self._pack.service,self._pack.subservice,self.__on_subscribe_response)

class LaserRanging(_BaseModule):
    def _init_module(self):
        self._pack.service = 0x63
        self._pack.subservice = 0x12
        self._distance = {}
    
    def __on_subscribe_response(self,pack):
        if len(pack.data)>1:
            self._distance[pack.subscribe_id] = utils.bits2float(pack.data[1:6])
    
    def get_distance(self,idx=None):
        subscribe_id = self.__subscribe(idx)
        if subscribe_id in self._distance:
            return self._distance[subscribe_id]
        return 0

    def __subscribe(self,idx=None):
        return super()._subscribe(idx or self._pack.idx,self._pack.service,self._pack.subservice,self.__on_subscribe_response)

class Flame(_BaseModule):
    def _init_module(self):
        self._pack.service = 0x63
        self._pack.subservice = 0x1b
        self._value = {}
    
    def __on_subscribe_response(self,pack):
        if len(pack.data)>1:
            self._value[pack.subscribe_id] = utils.bits2int8(pack.data[1:3])
    
    def get_value(self,idx=None):
        subscribe_id = self.__subscribe(idx)
        if subscribe_id in self._value:
            return self._value[subscribe_id]
        return 0

    def __subscribe(self,idx=None):
        return super()._subscribe(idx or self._pack.idx,self._pack.service,self._pack.subservice,self.__on_subscribe_response)

class PIRMotion(_BaseModule):
    def _init_module(self):
        self._pack.service = 0x63
        self._pack.subservice = 0x17
        self._status = {"is_moving":{},"count":{}}
    
    def __on_subscribe_response(self,pack):
        if len(pack.data)>1:
            if pack.data[0]==1:
                self._status["is_moving"][pack.subscribe_id] = pack.data[1]==1
            elif pack.data[0]==2:
                self._status["count"][pack.subscribe_id] = utils.bits2long(pack.data[1:6])
    
    def is_moving(self,idx=None):
        subscribe_id = self.__subscribe(idx)
        if subscribe_id in self._status["is_moving"]:
            return self._status["is_moving"][subscribe_id]
        return False
    
    def get_count(self,idx=None):
        subscribe_id = self.__subscribe(idx)
        if subscribe_id in self._status["count"]:
            return self._status["count"][subscribe_id]
        return 0

    def reset(self,idx=None):
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = self._pack.subservice
        pack.data = [0x3]
        self.call(pack)

    def __subscribe(self,idx=None):
        return super()._subscribe(idx or self._pack.idx,self._pack.service,self._pack.subservice,self.__on_subscribe_response)

class Magnetic(_BaseModule):
    def _init_module(self):
        self._pack.service = 0x63
        self._pack.subservice = 0x1e
        self._status = {"is_magnetic":{},"count":{}}
    
    def __on_subscribe_response(self,pack):
        if len(pack.data)>1:
            if pack.data[0]==1:
                self._status["is_magnetic"][pack.subscribe_id] = pack.data[1]==1
            elif pack.data[0]==2:
                self._status["count"][pack.subscribe_id] = utils.bits2long(pack.data[1:6])
    
    def is_magnetic(self,idx=None):
        subscribe_id = self.__subscribe(idx)
        if subscribe_id in self._status['is_magnetic']:
            return self._status["is_magnetic"][subscribe_id]
        return False
    
    def get_count(self,idx=None):
        subscribe_id = self.__subscribe(idx)
        if subscribe_id in self._status['count']:
            return self._status["count"][subscribe_id]
        return 0

    def reset(self,idx=None):
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = self._pack.subservice
        pack.data = [0x3]
        self.call(pack)

    def __subscribe(self,idx=None):
        return super()._subscribe(idx or self._pack.idx,self._pack.service,self._pack.subservice,self.__on_subscribe_response)

class Angle(_BaseModule):
    def _init_module(self):
        self._pack.service = 0x63
        self._pack.subservice = 0xe
        self._status = {"direction":{},"speed":{},"angle":{}}
    
    def __on_subscribe_response(self,pack):
        if len(pack.data)>1:
            if pack.data[0]==1:
                self._status["angle"][pack.subscribe_id] = utils.bits2long(pack.data[1:6])
            elif pack.data[0]==2:
                self._status["direction"][pack.subscribe_id] = pack.data[1]
            elif pack.data[0]==4:
                self._status["speed"][pack.subscribe_id] = utils.bits2long(pack.data[1:6])
    
    def get_angle(self,idx=None):
        subscribe_id = self.__subscribe(idx)
        if subscribe_id in self._status['angle']:
            return self._status["angle"][subscribe_id]
        return 0
    
    def get_direction(self,idx=None):
        subscribe_id = self.__subscribe(idx)
        if subscribe_id in self._status['direction']:
            return self._status["direction"][subscribe_id]
        return 0

    def get_speed(self):
        subscribe_id = self.__subscribe(idx)
        if subscribe_id in self._status['speed']:
            return self._status["speed"][subscribe_id]
        return 0

    def reset(self,idx=None):
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = self._pack.subservice
        pack.data = [0x3]
        self.call(pack)

    def __subscribe(self,idx=None):
        return super()._subscribe(idx or self._pack.idx,self._pack.service,self._pack.subservice,self.__on_subscribe_response)

class Motion(_BaseModule):
    def _init_module(self):
        self._pack.service = 0x63
        self._pack.subservice = 0x1a
        self._status = {"intensity":{},"acceleration_x":{},"acceleration_y":{},"acceleration_z":{},"velocity_x":{},"velocity_y":{},"velocity_z":{},"pitch":{},"roll":{}}
    
    def __on_subscribe_response(self,pack):
        if len(pack.data)>1:
            if pack.data[0]==1:
                self._status["intensity"][pack.subscribe_id] = pack.data[1]
            elif pack.data[0]==2:
                self._status["acceleration_x"][pack.subscribe_id] = utils.bits2float(pack.data[1:6])
            elif pack.data[0]==3:
                self._status["acceleration_y"][pack.subscribe_id] = utils.bits2float(pack.data[1:6])
            elif pack.data[0]==4:
                self._status["acceleration_z"][pack.subscribe_id] = utils.bits2float(pack.data[1:6])
            elif pack.data[0]==5:
                self._status["velocity_x"][pack.subscribe_id] = utils.bits2float(pack.data[1:6])
            elif pack.data[0]==6:
                self._status["velocity_y"][pack.subscribe_id] = utils.bits2float(pack.data[1:6])
            elif pack.data[0]==7:
                self._status["velocity_z"][pack.subscribe_id] = utils.bits2float(pack.data[1:6])
            elif pack.data[0]==8:
                self._status["pitch"][pack.subscribe_id] = utils.bits2short(pack.data[1:3])
            elif pack.data[0]==9:
                self._status["roll"][pack.subscribe_id] = utils.bits2short(pack.data[1:3])
    
    def get_intensity(self,idx=None):
        subscribe_id = self.__subscribe(idx)
        if subscribe_id in self._status['intensity']:
            return self._status["intensity"][subscribe_id]
        return 0
    
    def get_acceleration_x(self,idx=None):
        subscribe_id = self.__subscribe(idx)
        if subscribe_id in self._status['intensity']:
            return self._status["acceleration_x"][subscribe_id]
        return 0

    def get_acceleration_y(self,idx=None):
        subscribe_id = self.__subscribe(idx)
        if subscribe_id in self._status['intensity']:
            return self._status["acceleration_y"][subscribe_id]
        return 0

    def get_acceleration_z(self,idx=None):
        subscribe_id = self.__subscribe(idx)
        if subscribe_id in self._status['intensity']:
            return self._status["acceleration_z"][subscribe_id]
        return 0

    def get_velocity_x(self,idx=None):
        subscribe_id = self.__subscribe(idx)
        if subscribe_id in self._status['intensity']:
            return self._status["velocity_x"][subscribe_id]
        return 0

    def get_velocity_y(self,idx=None):
        subscribe_id = self.__subscribe(idx)
        if subscribe_id in self._status['intensity']:
            return self._status["velocity_y"][subscribe_id]
        return 0

    def get_velocity_z(self,idx=None):
        subscribe_id = self.__subscribe(idx)
        if subscribe_id in self._status['intensity']:
            return self._status["velocity_z"][subscribe_id]
        return 0

    def get_pitch(self,idx=None):
        subscribe_id = self.__subscribe(idx)
        if subscribe_id in self._status['intensity']:
            return self._status["pitch"][subscribe_id]
        return 0

    def get_roll(self,idx=None):
        subscribe_id = self.__subscribe(idx)
        if subscribe_id in self._status['intensity']:
            return self._status["roll"][subscribe_id]
        return 0

    def reset(self,idx=None):
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = self._pack.subservice
        pack.data = [0x4]
        self.call(pack)
        pack.data = [0x5]
        self.call(pack)
        pack.data = [0x6]
        self.call(pack)

    def __subscribe(self,idx=None):
        for i in range(9):
            super()._subscribe(self._pack.idx,self._pack.service,self.__on_subscribe_response,[0x1,NeuronPackData.TYPE_CHANGE,i+1,0,0,0,0,0])


class Color(_BaseModule):
    def _init_module(self):
        self._pack.service = 0x63
        self._pack.subservice = 0x11
        self._pack.on_response = self._on_parse
        self._color = {}
        self._intensity = {}
        self._reflect = {}
        
    def _on_parse(self, pack):
        super()._on_parse(pack)
        if len(pack.data)>1:
            if pack.data[0]==0x8:
                self._color = super()._run_callback({'red1':utils.bits2short(pack.data[1:3]),'green1':utils.bits2short(pack.data[3:5]),'blue1':utils.bits2short(pack.data[5:7]),'red2':utils.bits2short(pack.data[7:9]),'green2':utils.bits2short(pack.data[9:11]),'blue2':utils.bits2short(pack.data[11:13])})
            elif pack.data[0]==0x9:
                self._intensity = super()._run_callback([utils.bits2short(pack.data[1:3]),utils.bits2short(pack.data[3:5])])
            elif pack.data[0]==0xa:
                self._reflect = super()._run_callback([utils.bits2short(pack.data[1:3]),utils.bits2short(pack.data[3:5])])

    def get_color(self,idx=0,callback=None):
        idx = idx or self._pack.idx
        super().read(idx,self._pack.service,self._pack.subservice,[0x8],callback)
        return self._color

    def get_intensity(self,idx=0,callback=None):
        idx = idx or self._pack.idx
        super().read(idx,self._pack.service,self._pack.subservice,[0x9],callback)
        return self._intensity

    def get_reflect(self,idx=0,callback=None):
        idx = idx or self._pack.idx
        super().read(idx,self._pack.service,self._pack.subservice,[0xa],callback)
        return self._reflect

class GPIO(_BaseModule):
    def _init_module(self):
        self._pack.service = 0x68
        self._pack.subservice = 0x1
        self._callback = None
        self._value = {}

    def __on_parse(self,pack):
        super()._on_parse(pack)
        if pack.data[0]==3:
            self._value[pack.subscribe_id] = super()._run_callback(utils.bits2short(pack.data[1:3]))
        elif pack.data[0]==2:
            self._value[pack.subscribe_id] = super()._run_callback(pack.data[1])

    def digital_write(self,pin,level,idx=None):
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = self._pack.subservice
        pack.data = [0x1,pin,level]
        self.call(pack)

    def pwm_write(self,pin,pwm,freq=1000,idx=None):
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = self._pack.subservice
        pack.data = [0x4,pin]
        pack.data.extend(utils.float2bits(freq))
        pack.data.extend(utils.float2bits(pwm))
        self.call(pack)

    def digital_read(self,pin,idx=None,callback=None):
        idx = idx or self._pack.idx
        res = super().read(idx,self._pack.service,self._pack.subservice,[0x2,pin],callback)
        if idx in self._value:
            return self._value[idx]
        return 0

    def analog_read(self,pin,idx=None,callback=None):
        idx = idx or self._pack.idx
        res = super().read(idx,self._pack.service,self._pack.subservice,[0x3,pin],callback)
        if idx in self._value:
            return self._value[idx]
        return 0

    def enable(self,status=0,idx=None):
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = self._pack.subservice
        pack.data = [0x5,status]
        self.call(pack)

class PowerManager(_BaseModule):
    def _init_module(self):
        self._pack.service = 0x64
        self._pack.subservice = 0x9
        self._pack.on_response = self.__on_subscribe_response
        self._status = {}
        self._wireless = {}
        self._power = {}

    def __on_subscribe_response(self,pack):
        if len(pack.data)>1:
            if pack.data[0]==0x1:
                self._wireless[pack.subscribe_id] = pack.data[1]
                self._power[pack.subscribe_id] = pack.data[2]
            elif pack.data[0]==0x2:
                super()._on_parse(pack)
                self._status = super()._run_callback({'voltage':utils.bits2float(pack.data[1:6]),'current':utils.bits2float(pack.data[6:11])})

    def get_status(self,idx=None,callback=None):
        idx = idx or self._pack.idx
        res = super().read(idx,self._pack.service,self._pack.subservice,[0x2],callback)
        return self._status

    def is_wireless(self,idx=None):
        subscribe_id = self.__subscribe(idx)
        if subscribe_id in self._status['wireless']:
            return self._status["wireless"][subscribe_id]
        return True

    def is_power(self,idx=None):
        subscribe_id = self.__subscribe(idx)
        if subscribe_id in self._status['power']:
            return self._status["power"][subscribe_id]
        return True

    def __subscribe(self,idx=None):
        return super()._subscribe(idx or self._pack.idx,self._pack.service,self._pack.subservice,self.__on_subscribe_response,[0x7f,NeuronPackData.TYPE_CHANGE,0,0,0,0,0])

class Infrarer(_BaseModule):
    def _init_module(self):
        self._pack.service = 0x68
        self._pack.subservice = 0x2
    
    def send_message(self,msg="",idx=None):
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = self._pack.subservice
        pack.data = [0x2,len(msg)]
        pack.data.extend(utils.string2bytes(msg))
        self.call(pack)

    def record(self,index=0,time=3000,idx=None):
        if index>1:
            index=1
        if index<0:
            index=0
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = self._pack.subservice
        pack.data = [0x3]
        pack.data.extend(utils.long2bits(time))
        pack.data.append(index)
        self.call(pack)

    def send_record(self,index=0,idx=None):
        if index>1:
            index=1
        if index<0:
            index=0
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = self._pack.subservice
        pack.data = [0x4,index]
        self.call(pack)

class RGBLed(_BaseModule):
    def _init_module(self):
        self._pack.service = 0x65
        self._pack.subservice = 0x2
    
    def set_color(self,red,green,blue,idx=None):
        self._pack.idx = idx or self._pack.idx
        self._pack.data = [0x1]
        self._pack.data.extend(utils.ushort2bits(red))
        self._pack.data.extend(utils.ushort2bits(green))
        self._pack.data.extend(utils.ushort2bits(blue))
        self.call(self._pack)
    
class LedStrip(_BaseModule):
    def _init_module(self):
        self._pack.service = 0x65
        self._pack.subservice = 0x3
    
    def set_color(self,index,red,green,blue,idx=None):
        self._pack.idx = idx or self._pack.idx
        self._pack.data = [0x1,index]
        self._pack.data.extend(utils.ushort2bits(red))
        self._pack.data.extend(utils.ushort2bits(green))
        self._pack.data.extend(utils.ushort2bits(blue))
        self.call(self._pack)

class LedMatrix(_BaseModule):
    def _init_module(self):
        self._pack.service = 0x65
        self._pack.subservice = 0x9
    
    def set_pixel(self,index,red,green,blue,idx=None):
        self._pack.idx = idx or self._pack.idx
        self._pack.data = [0x2,index]
        self._pack.data.extend(utils.ushort2bits(red))
        self._pack.data.extend(utils.ushort2bits(green))
        self._pack.data.extend(utils.ushort2bits(blue))
        self.call(self._pack)
    
    def set_pixels(self,bits,red,green,blue,idx=None):
        self._pack.idx = idx or self._pack.idx
        self._pack.data = [0x1]
        for i in range(2):
            self._pack.data.extend(utils.long2bits(bits[i]))
        self._pack.data.extend(utils.ushort2bits(red))
        self._pack.data.extend(utils.ushort2bits(green))
        self._pack.data.extend(utils.ushort2bits(blue))
        self.call(self._pack)

    def set_string(self,msg,idx=None):
        self._pack.idx = idx or self._pack.idx
        l = len(msg)
        self._pack.data = [0x7]
        self._pack.data.extend(utils.ushort2bits(l))
        for i in range(l):
            self._pack.data.append(ord(msg[i]))
        self.call(self._pack)

class Servo(_BaseModule):
    BOTH_SERVOS = 1
    LEFT_SERVO = 2
    RIGHT_SERVO = 3
    def _init_module(self):
        self._pack.service = 0x62
        self._pack.subservice = 0xa
    
    def set_angle(self,angle,idx=None):
        self._pack.idx = idx or self._pack.idx
        angle = int(angle)
        self._pack.data = [0x1]
        self._pack.data.extend(utils.ushort2bits(angle))
        self.call(self._pack)
    
    def release(self,idx=None):
        self._pack.idx = idx or self._pack.idx
        self._pack.data = [0x6]
        self.call(self._pack)

class DCMotor(_BaseModule):
    def _init_module(self):
        self._pack.service = 0x62
        self._pack.subservice = 0x9
    
    def set_pwm(self,pwm,idx=None):
        self._pack.idx = idx or self._pack.idx
        self._pack.data = [0x1]
        self._pack.data.extend(utils.int2bits(pwm))
        self.call(self._pack)


class EncoderMotor(_BaseModule):
    def _init_module(self):
        self._pack.service = 0x62
        self._pack.subservice = 0x6
        self._stall = {}

    def set_speed(self,speed,idx=None):
        if speed>320:
            speed = 320
        if speed<-320:
            speed = -320
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = self._pack.subservice
        pack.data = [0x4]
        pack.data.extend(utils.ushort2bits(speed))
        self.call(pack)

    def set_pwm(self,pwm,idx=None):
        if pwm>255:
            pwm = 255
        if pwm<-255:
            pwm = -255
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = self._pack.subservice
        pack.data = [0x11]
        pack.data.extend(utils.ushort2bits(pwm))
        self.call(pack)

    def move_to(self,position,speed,idx=None):
        if speed>320:
            speed = 320
        if speed<0:
            speed = 0
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = self._pack.subservice
        pack.data = [0x2]
        pack.data.extend(utils.long2bits(position))
        pack.data.extend(utils.ushort2bits(speed))
        self.call(pack)

    def move(self,position,speed,idx=None):
        if speed>320:
            speed = 320
        if speed<0:
            speed = 0
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = self._pack.subservice
        pack.data = [0x3]
        pack.data.extend(utils.long2bits(position))
        pack.data.extend(utils.ushort2bits(speed))
        self.call(pack)

    def set_lock(self,lock,idx=None):
        if lock>1:
            lock = 1
        if lock<0:
            lock = 0
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = self._pack.subservice
        pack.data = [0x14,lock]
        self.call(pack)

    def set_zero(self,idx=None):
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = self._pack.subservice
        pack.data = [0x12]
        self.call(pack)

    def stop(self,idx = None):
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = self._pack.subservice
        pack.data = [0x5]
        self.call(pack)

    def __on_subscribe_response(self,pack):
        if pack.data[0]==0x27:
            self._status["stall"] = pack.data[1]==1
    
    def get_stall(self,idx=None):
        subscribe_id = self.__subscribe(idx)
        if subscribe_id in self._stall:
            return self._stall[subscribe_id]
        return False

    def __subscribe(self,idx=None):
        super()._subscribe(idx or self._pack.idx,self._pack.service,self._pack.subservice,[0x7d,NeuronPackData.TYPE_CHANGE,0,0,0,0,0])

class ExtDCMotor(_BaseModule):
    def _init_module(self):
        self._pack.service = 0x62
        self._pack.subservice = 0x7

    def reset(self,port,idx=None):
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = self._pack.subservice
        pack.data = [0x1,port]
        self.call(pack)

    def set_pwm(self,port,pwm,idx=None):
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = self._pack.subservice
        pack.data = [0x2,port]
        pack.data.extend(utils.ushort2bits(pwm))
        self.call(pack)

    def stop(self,port,idx=None):
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = self._pack.subservice
        pack.data = [0x3,port]
        self.call(pack)

class SmartServo(_BaseModule):
    def _init_module(self):
        self._pack.service = 0x60
        self._status = {"speed":{},"position":{},"temp":{},"current":{},"voltage":{}}

    def __on_parse(self,pack):
        super()._on_parse(pack)
        if pack.subservice == 0x41:
            super()._run_callback(pack.data[0])

    def __on_subscribe_response(self,pack):
        if pack.subservice==0x36:
            self._status["position"][pack.subscribe_id] = utils.bits2long(pack.data[0:5])
        elif pack.subservice==0x23:
            self._status["speed"][pack.subscribe_id] = int(utils.bits2float(pack.data[0:5])*10)/10
        elif pack.subservice==0x25:
            self._status["temp"][pack.subscribe_id] = int(utils.bits2float(pack.data[0:5])*100)/100
        elif pack.subservice==0x26:
            self._status["current"][pack.subscribe_id] = int(utils.bits2float(pack.data[0:5])*100)/100
        elif pack.subservice==0x27:
            self._status["voltage"][pack.subscribe_id] = int(utils.bits2float(pack.data[0:5])*100)/100

    def __subscribe(self,subservice,idx=None):
        idx = idx or self._pack.idx
        return super()._subscribe(idx,self._pack.service,subservice,self.__on_subscribe_response,[NeuronPackData.TYPE_CHANGE])

    def get_position(self,idx=None):
        subscribe_id = self.__subscribe(0x36,idx)
        if subscribe_id in self._status["position"]:
            return self._status["position"][subscribe_id]
        return 0

    def get_speed(self,idx=None):
        subscribe_id = self.__subscribe(0x23,idx)
        if subscribe_id in self._status["speed"]:
            return self._status["speed"][subscribe_id]
        return 0

    def get_voltage(self,idx=None):
        subscribe_id = self.__subscribe(0x27,idx)
        if subscribe_id in self._status["voltage"]:
            return self._status["voltage"][subscribe_id]
        return 0

    def get_current(self,idx=None):
        subscribe_id = self.__subscribe(0x26,idx)
        if subscribe_id in self._status["current"]:
            return self._status["current"][subscribe_id]
        return 0

    def get_temperature(self,idx=None):
        subscribe_id = self.__subscribe(0x25,idx)
        if subscribe_id in self._status["temp"]:
            return self._status["temp"][subscribe_id]
        return 0

    def set_lock(self,lock):
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = 0x16
        pack.data = [lock]
        self.call(pack)

    def set_led(self,red,green,blue):
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = 0x17
        pack.data = []
        pack.data.extend(utils.ushort2bits(red))
        pack.data.extend(utils.ushort2bits(green))
        pack.data.extend(utils.ushort2bits(blue))
        self.call(pack)

    def set_zero(self):
        pack = NeuronPackData()
        pack.service = self._pack.service
        pack.subservice = 0x30
        self.call(pack)

    def move_to(self,position,speed,idx=None):
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = 0x33
        pack.data = []
        pack.data.extend(utils.long2bits(position))
        pack.data.extend(utils.ushort2bits(speed))
        self.call(pack)

    def move(self,position,speed,idx=None):
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = 0x34
        pack.data = []
        pack.data.extend(utils.long2bits(position))
        pack.data.extend(utils.ushort2bits(speed))
        self.call(pack)
    
    def set_rotation(self,rotation,during,idx=None):
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = 0x2a
        pack.data = []
        pack.data.extend(utils.float2bits(rotation))
        pack.data.extend(utils.ushort2bits(during))
        self.call(pack)

    def set_pwm(self,pwm,idx=None):
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = 0x35
        pack.data = []
        pack.data.extend(utils.short2bits(pwm))
        self.call(pack)

    def set_absolute_angle_with_force(self,angle,speed,force=100,idx=None):
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = 0x38
        pack.data = []
        pack.data.extend(utils.long2bits(angle))
        pack.data.extend(utils.ushort2bits(speed))
        pack.data.extend(utils.ushort2bits(force))
        self.call(pack)

    def set_relative_angle_with_force(self,angle,speed,force=100,idx=None):
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = 0x39
        pack.data = []
        pack.data.extend(utils.long2bits(angle))
        pack.data.extend(utils.ushort2bits(speed))
        pack.data.extend(utils.ushort2bits(force))
        self.call(pack)

    def set_speed(self,speed,idx=None):
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = 0x3a
        pack.data = []
        pack.data.extend(utils.int2bits(speed))
        self.call(pack)

    def home(self,direction,speed,idx=None):
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = 0x43
        pack.data = [direction]
        pack.data.extend(utils.ushort2bits(speed))
        self.call(pack)

    def get_error(self,idx=None,callback=None):
        self._pack.on_response = self.__on_parse
        res = super().read(idx or self._pack.idx,self._pack.service,0x41,[],callback)
        return res[0]

    def clear_error(self,idx=None):
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = 0x42
        self.call(pack)

class Speaker(_BaseModule):
    def _init_module(self):
        self._pack.service = 0x66
        self._pack.subservice = 0x4

    def play_tone(self,hz,idx=None):
        self._pack.idx = idx or self._pack.idx
        self._pack.data = [0x2]
        self._pack.data.extend(utils.ushort2bits(hz))
        self.call(self._pack)
    
class FingertipPiano(_BaseModule):
    '''
        :description: Fingertip Piano
        :example:
        .. code-block:: python
            :linenos:

            piano = FingertipPiano(1)

    '''
    def _init_module(self):
        self._isExiting = False
        self._pack.service = 0x64
        self._pack.subservice = 0xf
        self._piano = {'touch_strength':{},'key':{},'touch':{},'joystick':{},'distance':{},'key_count':{},'last_touched':{},'last_pressed':{},'gesture':{}}

    def __on_parse(self, pack):
        pass

    def __on_subscribe_response(self,pack):
        if pack.data[0]==0x2:
            self._piano['key'][pack.subscribe_id] = pack.data[1:3]
        elif pack.data[0]==0x3:
            self._piano['touch'][pack.subscribe_id] = pack.data[1:5]
        elif pack.data[0]==0x6:
            self._piano['gesture'][pack.subscribe_id] = pack.data[1]
        elif pack.data[0]==0x5:
            self._piano['joystick'][pack.subscribe_id] = [utils.bits2int8(pack.data[1:3]),utils.bits2int8(pack.data[3:5])]
        elif pack.data[0]==0x7:
            self._piano['distance'][pack.subscribe_id] = pack.data[1]
        elif pack.data[0]==0x8:
            self._piano['touch_strength'][pack.subscribe_id] = [utils.bits2short(pack.data[1:3]),utils.bits2short(pack.data[3:5]),utils.bits2short(pack.data[5:7]),utils.bits2short(pack.data[7:9])]
        elif pack.data[0]==0x9:
            self._piano['key_count'][pack.subscribe_id] = [utils.bits2long(pack.data[1:6]),utils.bits2long(pack.data[6:11])]

    def __subscribe(self,type,idx=None,mode=NeuronPackData.TYPE_CHANGE):
        idx = idx or self._pack.idx
        return super()._subscribe(idx,self._pack.service,self._pack.subservice,self.__on_subscribe_response,[type,mode]+utils.long2bits(100))

    def count_pressed(self,index=1,idx=None):
        return self.get_count_pressed(index,idx)

    def get_count_pressed(self,index=1,idx=None):
        subscribe_id = self.__subscribe(0x78,idx)
        if subscribe_id in self._piano['key_count']:
            if index==1 or (type(index)==str and index.lower()=='a'):
                return self._piano['key_count'][subscribe_id][0]
            if index==2 or (type(index)==str and index.lower()=='b'):
                return self._piano['key_count'][subscribe_id][1]
        return 0

    def is_pressed(self,index=1,idx=None):
        subscribe_id = self.__subscribe(0x7f,idx)
        if subscribe_id in self._piano['key']:
            if index==1 or (type(index)==str and index.lower()=='a'):
                return self._piano['key'][subscribe_id][0] == 1
            if index==2 or (type(index)==str and index.lower()=='b'):
                return self._piano['key'][subscribe_id][1] == 1
        return False

    @property
    def is_obstacle_ahead(self):
        return self.distance<95

    @property
    def obstacle_moving(self):
        return self.gesture

    def last_pressed(self,idx=0):
        if idx==1:
            idx = 'A'
        elif idx==2:
            idx = 'B'
        if idx==0:
            if (self._piano['keyA']+self._piano['keyB'])==0 and self._piano['last_pressed']!=idx:
                self._piano['last_pressed']=idx
                return 0
        elif self._piano['key'+idx] and self._piano['last_pressed']!=idx:
            self._piano['last_pressed']=idx
            return 0
        return 1

    def touch_value(self,index=1,idx=None):
        subscribe_id = self.__subscribe(0x79,idx,NeuronPackData.TYPE_PERIOD)
        if subscribe_id in self._piano['touch_strength']:
            if index==1 or (type(index)==str and index.lower()=='a'):
                return int(self._piano['touch_strength'][subscribe_id][0]*100/256)
            elif index==2 or (type(index)==str and index.lower()=='b'):
                return int(self._piano['touch_strength'][subscribe_id][1]*100/256)
            elif index==3 or (type(index)==str and index.lower()=='c'):
                return int(self._piano['touch_strength'][subscribe_id][2]*100/256)
            elif index==4 or (type(index)==str and index.lower()=='d'):
                return int(self._piano['touch_strength'][subscribe_id][3]*100/256)
        return 0


    def is_touched(self,index=1,idx=None):
        subscribe_id = self.__subscribe(0x7e,idx)
        if subscribe_id in self._piano['touch']:
            if index==1 or (type(index)==str and index.lower()=='a'):
                return self._piano['touch'][subscribe_id][0]==1
            elif index==2 or (type(index)==str and index.lower()=='b'):
                return self._piano['touch'][subscribe_id][1]==1
            elif index==3 or (type(index)==str and index.lower()=='c'):
                return self._piano['touch'][subscribe_id][2]==1
            elif index==4 or (type(index)==str and index.lower()=='d'):
                return self._piano['touch'][subscribe_id][3]==1
        return False

    def last_touched(self,idx=0):
        if idx==0:
            if self._piano['touch4']+self._piano['touch1']+self._piano['touch2']+self._piano['touch3']==0 and self._piano['last_touched']!=idx:
                self._piano['last_touched']=idx
                return 0
        elif self._piano['touch'+str(idx)] and self._piano['last_touched']!=idx:
            self._piano['last_touched']=idx
            return 0
        return 1

    @property
    def gesture(self):
        return self.get_gesture()

    def get_gesture(self,idx=None):
        subscribe_id = self.__subscribe(0x7b,idx)
        if subscribe_id in self._piano['gesture']:
            return self._piano['gesture'][subscribe_id]
        return 0

    @property
    def distance(self):
        return self.get_distance()

    def get_distance(self,idx=None):
        subscribe_id = self.__subscribe(0x7a,idx,NeuronPackData.TYPE_PERIOD)
        if subscribe_id in self._piano['distance']:
            return self._piano['distance'][subscribe_id]
        return 0

    @property
    def ir_intensity(self):
        return 100 - self.distance

    def get_ir_intensity(self,idx):
        return 100 - self.get_distance(idx)

    @property
    def joystick_x(self):
        return self.get_joystick_x()

    def get_joystick_x(self,idx=None):
        subscribe_id = self.__subscribe(0x7c,idx,NeuronPackData.TYPE_PERIOD)
        if subscribe_id in self._piano['joystick']:
            return self._piano['joystick'][subscribe_id][0]
        return 0

    @property
    def joystick_y(self):
        return self.get_joystick_y()

    def get_joystick_y(self,idx=None):
        subscribe_id = self.__subscribe(0x7c,idx,NeuronPackData.TYPE_PERIOD)
        if subscribe_id in self._piano['joystick']:
            return self._piano['joystick'][subscribe_id][1]
        return 0

    @property
    def joystick_direction(self):
        if self.is_joystick_up:
            return self.DIRECTION_UP
        elif self.is_joystick_down:
            return self.DIRECTION_DOWN
        elif self.is_joystick_left:
            return self.DIRECTION_LEFT
        elif self.is_joystick_right:
            return self.DIRECTION_RIGHT
        return self.DIRECTION_HOME

    @property
    def is_joystick_up(self):
        return self.get_joystick_y()>30

    @property
    def is_joystick_down(self):
        return self.get_joystick_y()<-30

    @property
    def is_joystick_left(self):
        return self.get_joystick_x()<-30

    @property
    def is_joystick_right(self):
        return self.get_joystick_x()>30
    
    @property
    def DIRECTION_HOME(self):
        return 0

    @property
    def DIRECTION_UP(self):
        return 1

    @property
    def DIRECTION_DOWN(self):
        return 2

    @property
    def DIRECTION_LEFT(self):
        return 3

    @property
    def DIRECTION_RIGHT(self):
        return 4

    @property  
    def GESTURE_NONE(self):
        return 0

    @property  
    def GESTURE_WAVING(self):
        return 1
    
    @property  
    def GESTURE_MOVING_DOWN(self):
        return 2

    @property  
    def GESTURE_MOVING_UP(self):
        return 3

    @property
    def APPROACHING(self):
        return self.GESTURE_MOVING_DOWN

    @property
    def AWAY(self):
        return self.GESTURE_MOVING_UP

    def set_led(self,red=0,green=0,blue=0,idx=None):
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = self._pack.subservice
        pack.data = [0x1]
        pack.data.extend(utils.ushort2bits(red))
        pack.data.extend(utils.ushort2bits(green))
        pack.data.extend(utils.ushort2bits(blue))
        self.call(pack)

    def led_on(self,red=0,green=0,blue=0,idx=None):
        self.set_led(red,green,blue,idx)

    def led_off(self,idx=None):
        self.set_led(0,0,0,idx)

    def reset_button(self,index=2,idx=None):
        self.reset_pressed(index,idx)

    def reset_pressed(self,index=2,idx=None):
        if type(index)==str:
            if index.lower()=='a':
                index = 0
            elif index.lower()=='b':
                index = 1
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = self._pack.subservice
        pack.data = [0x11,index]
        self.call(pack)
    
    def calibrate(self,idx=None):
        pack = NeuronPackData()
        pack.idx = idx or self._pack.idx
        pack.service = self._pack.service
        pack.subservice = self._pack.subservice
        pack.data = [0x10]
        self.call(pack)