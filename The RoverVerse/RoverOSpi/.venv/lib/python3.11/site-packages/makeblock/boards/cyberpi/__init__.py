# -*- coding: utf-8 -*
from time import sleep
from ...modules.cyberpi import *
from ...boards.base import _BaseBoard
from ...protocols.PackData import HalocodePackData
from ...comm.SerialPort import SerialPort
from ...comm import mlink
MODE_REQUEST = 0
MODE_CHANGE = 1
MODE_PERIOD = 2

def connect(device=None,channel=None):
    return __CyberPi(device or channel).api

create = connect
from makeblock.modules.cyberpi import api_cyberpi_api
class __CyberPi(_BaseBoard):
    def __init__(self,device=None):
        self._type = _BaseBoard.CyberPi
        if device is not None:
            super().__init__(_BaseBoard.CyberPi,device)
            if device.type!='mlink':
                while not self.protocol.ready:
                    self.broadcast()
                    sleep(0.1)
                
            self.setTransferMode()
            self.module_auto = BaseModuleAuto(self)
            api_cyberpi_api.module_auto = self.module_auto
            self.api = api_cyberpi_api
            api_cyberpi_api.autoconnect()
        else:
            api_cyberpi_api.board = self
            self.api = api_cyberpi_api
            api_cyberpi_api.autoconnect()
            
    def setTransferMode(self):
        # pack = HalocodePackData()
        # pack.type = HalocodePackData.TYPE_SCRIPT
        # pack.mode = HalocodePackData.TYPE_RUN_WITHOUT_RESPONSE
        # pack.script = "global_objects.communication_o.enable_protocol(global_objects.communication_o.REPL_PROTOCOL_GROUP_ID)"
        # self.call(pack)
        # sleep(1)
        # self.repl('import communication')
        # self.repl('communication.bind_passthrough_channels("uart0", "uart1")')
        # sleep(1)

        pass

    def broadcast(self):
        pack = HalocodePackData()
        pack.type = HalocodePackData.TYPE_SCRIPT
        pack.mode = HalocodePackData.TYPE_RUN_WITH_RESPONSE
        pack.script = "cyberpi.get_bri()"
        while not self.protocol.ready:
            self.call(pack)
            sleep(0.3)
