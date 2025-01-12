# -*- coding: utf-8 -*
from makeblock.modules.rj25 import *
from makeblock.boards.megapi import __MegaPi
from makeblock.boards.base import _BaseBoard

board = None
def connect(device=None,channel=None):
    return __MegaPiPro(device or channel)

create = connect

class __MegaPiPro(__MegaPi):
    def __init__(self,device=None):
        self._type = _BaseBoard.MegaPiPro
        if not device is None:
            super().__init__(_BaseBoard.MegaPiPro,device)