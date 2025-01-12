# -*- coding: utf-8 -*
from ...protocols import HalocodeProtocol,MegaPiProtocol,NeuronProtocol
from ...protocols import HalocodePackData
from ...comm import mlink
from ...comm.SerialPort import SerialPort
from time import sleep
import makeblock
class _BaseBoard:
    MegaPi = "megapi"
    MeOrion = "meorion"
    mCore = "mcore"
    MeAuriga = "meauriga"
    mBuild = "mbuild"
    Halocode = "halocode"
    MegaPiPro = "megapipro"
    CyberPi = "cyberpi"
    Neuron = "neuron"
    _dev = None
    _type = None
    broadcast = None
    def __init__(self,type,device):
        self.__setup(type,device)

    def __setup(self,type,device):
        self._type = type
        self._dev = device
        if type==_BaseBoard.Halocode or type==_BaseBoard.CyberPi:
            self._protocol = HalocodeProtocol()
        elif type==_BaseBoard.mBuild or type==_BaseBoard.Neuron :
            self._protocol = NeuronProtocol()
        else:
            self._protocol = MegaPiProtocol()
        self._dev.setup(self._protocol.on_parse)
        self._protocol.setup(self)
        self._responses = []
        self._subscribe_responses = []
        self._subscribes = {}

    def _autoconnect(self):
        if self._dev is None:
            self.connect()
            sleep(1)
            if not self.broadcast is None:
                self.broadcast()
    
    def connect(self,device=None,channel=None):
        if type(device)==int:
            channel = device
        if channel is None:
            channels = mlink.list()
            if len(channels)>0:
                self._dev = mlink.connect(channels[0])
                return self.__setup(self._type,self._dev)
        else:
            self._device = mlink.connect(channel)
            return self.__setup(self._type,self._dev)
        if device is None:
            ports = [port[0] for port in SerialPort.list() if port[2] != 'n/a' and port[2].find('1A86:7523')>0 ]
            if len(ports)>0:
                device = SerialPort(ports[0])
                self._dev = device
                return self.__setup(self._type,self._dev)
    
    def add_thread(self,thread):
        makeblock.add_thread(thread)

    def call(self,pack):
        self._dev.send(pack.to_buffer())

    def request(self,pack):
        if pack.idx==0:
            pack.idx = self._protocol.next_idx
        self._subscribes[pack.subscribe_id] = True
        self._responses.append(pack)
        self._dev.send(pack.to_buffer())

    def repl(self,script):
        buf = bytearray()
        buf.extend(map(ord, script+"\r\n"))
        self._dev.send(buf)

    def subscribe(self,pack):
        pack.subscribe_key = self._protocol.next_subscribe_key
        self._subscribe_responses.append(pack)
        self._dev.send(pack.to_buffer())

    def unsubscribe(self,pack):
        subscribe_pack = self.find_subscribe_response(pack)
        self.remove_subscribe_response(subscribe_pack)
        self._dev.send(pack.to_buffer())

    def on_response(self,pack):
        resp = self.find_response(pack)
        if not resp is None:
            pack.subscribe_id = resp.subscribe_id
            resp.on_response(pack)

    def on_subscribe_response(self,pack):
        resp = self.find_subscribe_response(pack)
        if not resp is None:
            pack.value_name = resp.value_name
            resp.on_response(pack)

    def find_response(self,pack):
        for i in range(len(self._responses)):
            if self._protocol.check_response(pack,self._responses[i]):
                return self._responses[i]
        return None

    def remove_response(self,pack):
        if pack in self._responses:
            self._responses.remove(pack)
    
    def find_subscribe_response(self,pack):
        for i in range(len(self._subscribe_responses)):
            if self._protocol.check_subscribe_response(pack,self._subscribe_responses[i]):
                return self._subscribe_responses[i]
        return None

    def remove_subscribe_response(self,pack):
        if pack in self._subscribe_responses:
            self._subscribe_responses.remove(pack)

    def enable_sending(self):
        self._dev.enable_sending()

    def disable_sending(self):
        self._dev.disable_sending()

    def _find_subscribe(self,subscribe_id):
        if subscribe_id in self._subscribes:
            return True
        return False

    @property
    def protocol(self):
        return self._protocol

    @property
    def type(self):
        return self._type
    
from makeblock.utils import bytes2string
class TestEngine():
    def __init__(self,device,protocol):
        self._dev = device
        self._protocol = protocol
        self._dev.setup(self._protocol.on_parse)
        self._protocol.setup(self)
        self._callback = None

    def on_response(self,resp):
        if not self._callback is None:
            msg = bytes2string(resp)
            try:
                self._callback(eval(msg))
            except(RuntimeError, SyntaxError, TypeError, NameError):
                self._callback(eval("{'err':'json parse err'}"))
        
    def setCallback(self,callback):
        self._callback = callback