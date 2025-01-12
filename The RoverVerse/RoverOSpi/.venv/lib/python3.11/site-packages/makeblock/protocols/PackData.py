# -*- coding: utf-8 -*
import makeblock.utils

class NeuronPackData:
    TYPE_REQUEST = 0
    TYPE_CHANGE = 1
    TYPE_PERIOD = 2
    def __init__(self,pack=[]):
        self._type = NeuronPackData.TYPE_REQUEST
        self._subscribe_key = 0
        self._subscribe_id = 0
        self._subscribe_value = 0
        self._header = 0xf0
        self._idx = 0x0
        self._service = 0x0
        self._port = 0x0
        self._slot = 0x0
        self._subservice = 0x0
        self._data = []
        self._checksum = 0x0
        self._footer = 0xf7
        self._on_response = ""
        end = len(pack)
        if(end > 0):
            self._header = pack[0]
            self._idx = pack[1]
            self._service = pack[2]
            self._subservice = pack[3]
            self._data = pack[4:end-2]
            if pack[end-2]>0:
                self._checksum = pack[end-2]
            else:
                self._checksum = self.checksum
            self._footer = pack[end-1]

    def to_buffer(self):
        bytes = bytearray()
        bytes.append(self._header)
        bytes.append(self._idx)
        bytes.append(self._service)
        bytes.append(self._subservice)
        if not self._data is None:
            _len = len(self._data)
            if _len>0:
                for i in range(_len):
                    bytes.append(self._data[i])
        bytes.append(self.checksum)
        bytes.append(self._footer)
        return bytes

    @property
    def checksum(self):
        sum = self._idx+self._service+self._subservice
        if not self._data is None:
            for i in self._data:
                sum += i
        sum &= 0x7f
        return sum

    @property
    def subscribe_id(self):
        return self._subscribe_id

    @subscribe_id.setter
    def subscribe_id(self,value):
        self._subscribe_id = value

    @property
    def subscribe_key(self):
        return self._subscribe_key

    @subscribe_key.setter
    def subscribe_key(self,value):
        self._subscribe_key = value

    @property
    def subscribe_value(self):
        return self._subscribe_value

    @subscribe_value.setter
    def subscribe_value(self,value):
        self._subscribe_value = value

    @property
    def service(self):
        return self._service

    @service.setter
    def service(self,value):
        self._service = value

    @property
    def subservice(self):
        return self._subservice

    @subservice.setter
    def subservice(self,value):
        self._subservice = value
        
    @property
    def idx(self):
        return self._idx

    @idx.setter
    def idx(self,value):
        self._idx = value
    
    @property
    def type(self):
        return self._type

    @type.setter
    def type(self,value):
        self._type = value

    @property
    def port(self):
        return self._port

    @port.setter
    def port(self,value):
        self._port = value

    @property
    def slot(self):
        return self._slot

    @slot.setter
    def slot(self,value):
        self._slot = value

    @property
    def data(self):
        return self._data

    @data.setter
    def data(self,value):
        self._data = value

    @property
    def on_response(self):
        return self._on_response

    @on_response.setter
    def on_response(self,value):
        self._on_response = value
    
    @staticmethod
    def broadcast():
        pack = NeuronPackData([0xf0,0xff,0x10,0x0,0xf,0xf7])
        return pack

class HalocodePackData():
    TYPE_RUN_WITHOUT_RESPONSE = 0x0
    TYPE_RUN_WITH_RESPONSE = 0x1
    TYPE_RESET = 0x2
    TYPE_RUN_IMDT_WITE_RESPONSE = 0x3
    TYPE_SUBSCRIBE = 0x29
    TYPE_SCRIPT = 0x28
    TYPE_ONLINE = 0xd
    def __init__(self,buf=[]):
        self._type = HalocodePackData.TYPE_SCRIPT
        self._mode = HalocodePackData.TYPE_RUN_WITHOUT_RESPONSE
        self._value_name = ""
        self._header = 0xf3
        self._datalen = 0x0
        self._idx = 0
        self._type = 0x0
        self._data = []
        self._checksum = 0x0
        self._footer = 0xf4
        self._on_response = ""
        self._event_cb = None
        self._subscribe_key = 0
        self._subscribe_value = 0
        self._subscribe_id = 0
        self._script = ""
        end = len(buf)
        if(end > 7):
            self._header = buf[0]
            self._datalen = buf[2]+buf[3]*256
            self._type = buf[4]
            self._mode = buf[5]
            if self._type==HalocodePackData.TYPE_SCRIPT:
                self._idx = buf[6]+(buf[7]<<8)
                self._data = buf[7:end-2]
            elif self._type==HalocodePackData.TYPE_SUBSCRIBE:
                self._data = buf[5:end-2]
            elif self._type==HalocodePackData.TYPE_ONLINE:
                self._idx = buf[5]+(buf[6]<<8)
            if buf[end-2]>0:
                self._checksum = buf[end-2]
            else:
                self._checksum = self.checksum
            self._footer = buf[end-1]

    def to_buffer(self):
        bytes = bytearray()
        bytes.append(self._header)
        datalen = len(self._data)+4
        if self._type == HalocodePackData.TYPE_ONLINE:
            datalen-=1
        bytes.append((((datalen>>8)&0xff)+(datalen&0xff)+0xf3)&0xff)
        bytes.append(datalen&0xff)
        bytes.append((datalen>>8)&0xff)
        bytes.append(self._type)
        if self._type != HalocodePackData.TYPE_ONLINE:
            bytes.append(self._mode)
        bytes.append(self._idx&0xff)
        bytes.append((self._idx>>8)&0xff)
        for i in range(len(self._data)):
            bytes.append(self._data[i])
        bytes.append(self.checksum)
        bytes.append(self._footer)
        return bytes

    @property
    def checksum(self):
        sum = self._type+self._mode+((self._idx>>8)&0xff)+(self._idx&0xff)
        for i in range(len(self._data)):
            sum += self._data[i]
        return sum&0xff

    @property
    def script(self):
        return self._script

    @script.setter
    def script(self,value):
        self._script = value

        script_data = bytearray(value, "utf8")

        datalen = len(script_data)
        self._data = [datalen&0xff,(datalen>>8)&0xff]
        self._data.extend(script_data)

    @property
    def subscribe_key(self):
        return self._subscribe_key

    @subscribe_key.setter
    def subscribe_key(self,value):
        self.script = self.script.format(value)
        self._subscribe_key = value

    @property
    def subscribe_value(self):
        return self._subscribe_value

    @subscribe_value.setter
    def subscribe_value(self,value):
        self._subscribe_value = value

    @property
    def value_name(self):
        return self._value_name

    @value_name.setter
    def value_name(self,value):
        self._value_name = value

    @property
    def subscribe_id(self):
        return self._subscribe_id

    @subscribe_id.setter
    def subscribe_id(self,value):
        self._subscribe_id = value

    @property
    def type(self):
        return self._type

    @type.setter
    def type(self,value):
        self._type = value

    @property
    def mode(self):
        return self._mode

    @mode.setter
    def mode(self,value):
        self._mode = value

    @property
    def datalen(self):
        return self._datalen

    @datalen.setter
    def datalen(self,value):
        self._datalen = value

    @property
    def idx(self):
        return self._idx

    @idx.setter
    def idx(self,value):
        self._idx = value

    @property
    def data(self):
        return self._data

    @data.setter
    def data(self,value):
        self._data = value

    @property
    def on_response(self):
        return self._on_response

    @on_response.setter
    def on_response(self,value):
        self._on_response = value
    
    @staticmethod
    def broadcast():
        return HalocodePackData([0xf3, 0xf6, 0x03, 0x0, 0x0d, 0x0, 0x01, 0x0e, 0xf4])

    @staticmethod
    def repl_mode():
        pack =  HalocodePackData()
        pack.type = HalocodePackData.TYPE_SCRIPT
        pack.mode = HalocodePackData.TYPE_RUN_WITH_RESPONSE
        pack.script = "global_objects.communication_o.enable_protocol(global_objects.communication_o.REPL_PROTOCOL_GROUP_ID)"
        return pack

class MegaPiPackData():
    ACTION_GET = 1
    ACTION_RUN = 2
    ACTION_RESET = 4
    ACTION_START = 5
    def __init__(self,pack=[]):
        self._headers = [0xff,0x55]
        self._idx = 0x0
        self._action = MegaPiPackData.ACTION_RUN
        self._port = 0
        self._slot = 0
        self._module = 0
        self._subscribe_id = 0
        self._data = []
        self._checksum = 0x0
        self._on_response = ""
        self._footer = 0xa
        end = len(pack)
        if(end > 0):
            self._headers = pack[0:2]
            self._idx = pack[2]
            self._data = pack[3:end-2]
            self._footer = pack[end-1]

    def to_buffer(self):
        bytes = bytearray()
        bytes.extend(self._headers)
        bytes.append(0)
        bytes.append(self._idx)
        bytes.append(self._action)
        bytes.append(self._module)
        for i in range(len(self._data)):
            bytes.append(self._data[i])
        bytes.append(self._footer)
        bytes[2] = len(bytes)-3
        return bytes

    @property
    def checksum(self):
        sum = self._idx+self._action+self._module
        for i in range(len(self._data)):
            sum += self._data[i]
        sum &= 0x7f
        return sum

    @property
    def module(self):
        return self._module

    @module.setter
    def module(self,value):
        self._module = value

    @property
    def idx(self):
        return self._idx

    @idx.setter
    def idx(self,value):
        self._idx = value
    
    @property
    def port(self):
        return self._port

    @port.setter
    def port(self,value):
        self._port = value

    @property
    def slot(self):
        return self._slot

    @slot.setter
    def slot(self,value):
        self._slot = value
        
    @property
    def action(self):
        return self._action

    @action.setter
    def action(self,value):
        self._action = value

    @property
    def data(self):
        return self._data

    @data.setter
    def data(self,value):
        self._data = value

    @property
    def subscribe_id(self):
        return self._subscribe_id

    @subscribe_id.setter
    def subscribe_id(self,value):
        self._subscribe_id = value

    @property
    def on_response(self):
        return self._on_response

    @on_response.setter
    def on_response(self,value):
        self._on_response = value
    