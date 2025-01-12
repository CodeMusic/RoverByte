# -*- coding: utf-8 -*
from ...modules.rj25n.modules import *
from ...boards.base import _BaseBoard
from ...comm.SerialPort import SerialPort
from ...comm import mlink
from time import sleep
PORT1 = 1
PORT2 = 2
PORT3 = 3
PORT4 = 4
SLOT1 = 1
SLOT2 = 2
M1 = 1
M2 = 2

board = None
def connect(device=None,channel=None):
    global board
    if type(device)==int:
        channel = device
    if not board is None:
        return board
    if channel is None:
        channels = mlink.list()
        if len(channels)>0:
            device = mlink.connect(channels[0])
            board = Modules(device)
            return board
    else:
        device = mlink.connect(channel)
        board = Modules(device)
        return board
    if device is None:
        ports = [port[0] for port in SerialPort.list() if port[2] != 'n/a' and port[2].find('1A86:7523')>0]
        if len(ports)>0:
            device = SerialPort(ports[0])
            board = Modules(device)
            return board
    return Modules(device)

create = connect

class Modules(_BaseBoard):
    def __init__(self,device):
        super().__init__(_BaseBoard.Neuron,device)

    def RGBLed(self,port,slot):
        return RGBLed(self,port,slot)
    
    def RGBLedOnBoard(self):
        return RGBLed(self,7,2)
    
    def ButtonOnBoard(self):
        return ButtonOnBoard(self)

    def Ultrasonic(self,port=1):
        return Ultrasonic(self,port)