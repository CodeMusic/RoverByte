# -*- coding: utf-8 -*
import glob
import sys
import threading
from multiprocessing import Array, Manager, Process
from time import ctime, sleep
import serial.tools.list_ports
import serial
import makeblock.utils
import makeblock

def connect(port,baudrate=115200):
    """
    .. code-block:: python
        :linenos:

        from makeblock import SerialPort
        from makeblock import MegaPi

        uart = SerialPort.connect("COM3")
        board = MegaPi.connect(uart)

    """
    uart = SerialPort(port,baudrate)
    return uart

create = connect

class SerialPort():
    """
    """
    def __init__(self, port="/dev/ttyAMA0", baudrate=115200, timeout=1):
        self.exiting = False
        self._is_sending = True
        self._responses = []
        self._queue = []
        try:
            self._ser = serial.Serial(port,baudrate)
            self._ser.timeout = 0.01
            sleep(2)
            self._thread = threading.Thread(target=self._on_read,args=(self._callback,))
            # self._thread.daemon = True
            self._thread.start()
            makeblock.add_port(self)
        except Exception as e:
            # print(e)
            pass
            # //\033[1;33;44m

    def setup(self,callback):
        self._responses.append(callback)

    @property
    def type(self):
        return "uart"
        
    def _callback(self,received):
        for method in self._responses:
            method(received)

    def _on_read(self,callback):
        while True:
            if self.exiting:
                break
            if self._is_sending:
                self.__sending()
                    
            if self.is_open():
                # if self.in_waiting()>0:
                buf = self.read()
                if len(buf)==1:
                    callback(buf[0])
            # else:    
            #     sleep(0.001)
                
    def send(self,buffer):
        if self.is_open():
            # makeblock.utils.print_hex(buffer)
            # self._ser.write(buffer)
            self._queue.append(buffer)
        # sleep(0.002)

    def __sending(self):
        if len(self._queue)>0:
            if self.is_open():
                buf = self._queue[0]
                self._ser.write(buf)
                self._queue.pop(0)

    def read(self):
        return self._ser.read()
        # try:
        #     return self._ser.read()
        # except serial.serialutil.SerialException:
        #     return []

    def enable_sending(self):
        self._is_sending = True

    def disable_sending(self):
        self._is_sending = False
        
    def is_open(self):
        return self._ser.isOpen()

    def in_waiting(self):
        return self._ser.inWaiting()

    def close(self):
        self._ser.close()

    def exit(self):
        self.exiting = True
        self._thread.join()
        self.close()

    @staticmethod
    def list():
        """
        获取串口列表

        .. code-block:: python
            :linenos:

            from makeblock import SerialPort
            print(SerialPort.list())

        :param: 无
        :return: 串口列表
        """
        return serial.tools.list_ports.comports()
