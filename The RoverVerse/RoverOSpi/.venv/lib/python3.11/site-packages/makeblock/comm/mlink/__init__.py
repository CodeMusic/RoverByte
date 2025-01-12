#!/usr/bin/env python
# -*- coding: utf-8 -*-
# @Author  : sky.li (13751020500@163.com)
# @Desc    : mlink 接口

import asyncio
import threading
import makeblock
from time import sleep
mlink_ready = False
try:
    import mlink as mlink_client
    mlink_ready = True
except ImportError:
    pass

def connect(channel=0):
    return mlink(channel)

def list():
    return mlink.list()
    
create = connect

class mlink():
    def __init__(self,channel=0):
        self._channel_id = channel
        self._exiting = False
        self._responses = []
        self._channel = None
        self._loop = asyncio.get_event_loop()
        self._thread = threading.Thread(target=self._on_read,args=(self._callback,))
        self._thread.start()
        sleep(2)
        
    def setup(self,callback):
        self._responses.append(callback)

    @property
    def type(self):
        return "mlink"

    def _callback(self,received):
        for method in self._responses:
            method(received)

    async def _on_loop(self):
        await mlink_client.start()
        asyncio.sleep(2)
        self._channel = mlink_client.getChannel(self._channel_id)
        makeblock.add_port(self)
        while True:
            if self._exiting:
                break
            try:
                buf = await asyncio.wait_for(self._channel.recv(),1)
                for i in range(len(buf)):
                    self._callback(buf[i])
            except asyncio.TimeoutError:
                # print('timeout!')
                pass

    def _on_read(self,callback):
        self._loop.run_until_complete(self._on_loop())
                
    def send(self,buffer):
        if not self._channel is None:
            l = []
            for i in range(len(buffer)):
                l.append(buffer[i])
            self._channel.send(l)

    def close(self):
        self._thread.join()
        self._loop.call_soon_threadsafe(mlink_client.stop())

    def exit(self):
        print("exit")
        self._exiting = True
        self.close()

    @staticmethod
    def list():
        global mlink_ready
        if mlink_ready:
            return [0]
        else:
            return []

