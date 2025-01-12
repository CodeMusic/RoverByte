# -*- coding: utf-8 -*
from makeblock.protocols.PackData import *
import makeblock.utils
class Protocol():
    _idx = 1
    _subscribe_idx = 1
    def __init__(self):
        self._engine = None
            
    def setup(self,engine):
        self._engine = engine

    def on_response(self,pack):
        self._engine.on_response(pack)

    def on_subscribe_response(self,pack):
        self._engine.on_subscribe_response(pack)  

    def check_response(self,pack):
        pass

    @property
    def next_idx(self):
        return 0

    @property
    def next_subscribe_key(self):
        self._subscribe_idx += 1
        if self._subscribe_idx>0xffff:
            self._subscribe_idx = 1
        return self._subscribe_idx
        
class NeuronProtocol(Protocol):
    def __init__(self):
        self._buffer = []

    def on_parse(self,byte):
        if byte==0xf0:
            self._engine.disable_sending()
            self._buffer = []
        # print('%#c'%byte)
        # print('%#x'%byte)
        self._buffer.append(byte)
        bufferLength = len(self._buffer)
        if bufferLength >= 2:
            if self._buffer[-1]==0xf7 and self._buffer[0]==0xf0:
                # print(self._buffer)
                self._engine.enable_sending()
                self.on_response(NeuronPackData(self._buffer))
                self._buffer = []
    
    def check_response(self,resp_pack,list_pack):
        # print(list_pack.idx,resp_pack.idx,list_pack.service,resp_pack.service,list_pack.subservice,resp_pack.subservice)
        return (list_pack.idx==resp_pack.idx) and (list_pack.service==resp_pack.service) and (list_pack.subservice==resp_pack.subservice)

class HalocodeProtocol(Protocol):
    def __init__(self):
        self._buffer = []
        self._isReceiving = False
        self._datalen = 0
        self._idx = 1
        self._ready = False
    
    def on_parse(self,byte):
        self._buffer.append(byte)
        if len(self._buffer) > 3 \
           and ((self._buffer[-1] + self._buffer[-2] + self._buffer[-4])&0xff) == self._buffer[-3] \
           and self._buffer[-4] == 0xf3:
            self._buffer = [0xf3,self._buffer[-3],self._buffer[-2],self._buffer[-1]]
            self._datalen = self._buffer[-2] + (self._buffer[-1]<<8)
            self._isReceiving = True
        if self._isReceiving:
            # 6 = 4(head) + 2(end)
            if (self._datalen == len(self._buffer) - 6) and self._buffer[0]==0xf3:
                self._ready = True
                pack = HalocodePackData(self._buffer)        
                if pack.type==HalocodePackData.TYPE_SCRIPT:
                    self.on_response(pack)
                elif pack.type==HalocodePackData.TYPE_SUBSCRIBE:
                    self.on_subscribe_response(pack)
                self._buffer = []
                self._datalen = 0
                self._isReceiving = False

    def on_subscribe_response(self,pack):
        # data_str = "".join([ chr(i) for i in pack.data[3:len(pack.data)]])
        data_str = str(bytes(pack.data[3:len(pack.data)]), "utf8")
        try:
            res = eval(data_str)
            for i in res:
                pack.subscribe_key = i
                pack.subscribe_value = res[i]
                super().on_subscribe_response(pack)
        except:
            pass

    def check_response(self,resp_pack,list_pack):
        return list_pack.idx==resp_pack.idx

    def check_subscribe_response(self,resp_pack,list_pack):
        return list_pack.subscribe_key==resp_pack.subscribe_key

    @property
    def ready(self):
        return self._ready

    @property
    def next_idx(self):
        self._idx += 1
        if self._idx>0xffff:
            self._idx = 1
        return self._idx   

class MegaPiProtocol(Protocol):
    def __init__(self):
        self._buffer = []
        self._idx = 1

    def on_parse(self,byte):
        if len(self._buffer)>0 and self._buffer[-1]==0xff and byte==0x55:
            self._buffer = [0xff]
        self._buffer.append(byte)
        bufferLength = len(self._buffer)
        if bufferLength >= 3:
            if self._buffer[-1]==0xa and self._buffer[-2]==0xd:
                self.on_response(MegaPiPackData(self._buffer))
                self._buffer = []
    
    def check_response(self,resp_pack,list_pack):
        return list_pack.idx==resp_pack.idx

    @property
    def next_idx(self):
        self._idx += 1
        if self._idx>0xff:
            self._idx = 1
        return self._idx  

class TestProtocol(Protocol):
    def __init__(self):
        self._buffer = []
        self._idx = 1

    def on_parse(self,byte):
        self._buffer.append(byte)
        bufferLength = len(self._buffer)
        if bufferLength >= 3:
            if self._buffer[-1]==0xa and self._buffer[-2]==0xd:
                self.on_response(self._buffer)
                self._buffer = []