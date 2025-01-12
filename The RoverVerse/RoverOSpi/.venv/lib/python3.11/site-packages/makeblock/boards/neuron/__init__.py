# -*- coding: utf-8 -*
from makeblock.boards.neuron.modules import *
from makeblock.boards.base import _BaseBoard
import makeblock.protocols as Protocols
from makeblock.protocols.PackData import NeuronPackData


MODE_REQUEST = 0
MODE_CHANGE = 1
MODE_PERIOD = 2

def connect(device):
    return Neuron(device)

create = connect

class Neuron(_BaseBoard):
    def __init__(self,device):
        super().__init__(device,Protocols.NeuronProtocol())
        self.broadcast()

    def broadcast(self):
        self._dev.send(NeuronPackData.broadcast().to_buffer())