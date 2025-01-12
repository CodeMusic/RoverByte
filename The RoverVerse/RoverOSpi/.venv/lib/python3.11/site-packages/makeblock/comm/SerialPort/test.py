import sys
import serial
import threading
import signal
from time import sleep,time
ser = serial.Serial("/dev/cu.wchusbserial1410",115200)
exiting = False
total = 0
buf = bytearray()
sent_count = 0
received_count = 0
start_time = time()
def __exiting(signal, frame):
    global exiting,sent_count,received_count
    during = time() - start_time
    print("time:",during,"sent:",sent_count/during/1024,"recv:",received_count/during/1024)
    exiting=True
    sys.exit(0)

signal.signal(signal.SIGINT, __exiting)
len = 100
for i in range(len):
    buf.append(0x31+i)
def on_read():
    global received_count,exiting,ser
    while True:
        if exiting:
            break    
        if ser.isOpen:
            len = ser.inWaiting()
            for i in range(len):
                ser.read()
            received_count = received_count+len
            sleep(0.0001)
thread = threading.Thread(target=on_read,args=())
thread.start()
while True:
    ser.write(buf)
    sent_count = sent_count+len
    sleep(0.001)
    