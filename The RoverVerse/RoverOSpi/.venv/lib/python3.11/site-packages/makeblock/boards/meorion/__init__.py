# -*- coding: utf-8 -*
from makeblock.modules.rj25 import *
from makeblock.boards.megapi import __MegaPi
from makeblock.boards.base import _BaseBoard

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

def connect(device=None, channel=None):
    return __MeOrion(device or channel)

create = connect

class __MeOrion(__MegaPi):
    def __init__(self,device=None):
        self._type = _BaseBoard.MeOrion
        if not device is None:
            super().__init__(_BaseBoard.MeOrion,device)
