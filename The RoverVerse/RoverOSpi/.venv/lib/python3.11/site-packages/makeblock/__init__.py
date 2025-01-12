#from .comm import SerialPort
#from .comm import mlink


import sys
import signal

_ports = []
_threads = []

def add_port(port):
    global _ports
    _ports.append(port)

def add_thread(thread):
    global _threads
    thread.daemon = True
    _threads.append(thread)

def quit():
    __exiting(0,0)
    
def __exiting(signal, frame):
    global _ports
    global _threads
    print("exiting")
    for port in _ports:
        port.exit()
    for thread in _threads:
        thread.exit()
    sys.exit(0)

signal.signal(signal.SIGINT, __exiting)

from .boards import *

