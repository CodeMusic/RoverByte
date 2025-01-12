# -*- coding: utf-8 -*
import struct

def print_hex(bytes):
    l = [hex(int(i)) for i in bytes]
    print("hex:"," ".join(l))

def long2bits(val):
    return [val&0x7f,(val>>7)&0x7f,(val>>14)&0x7f,(val>>21)&0x7f,(val>>28)&0x7f]

def ushort2bits(val):
    return [val&0x7f,(val>>7)&0x7f]

def short2bits(val):
    return [val&0x7f,(val>>7)&0x7f,(val>>14)&0x7f]

def float2bits(val):
    data = float2bytes(val)
    lval = bytes2long(data)
    bits = long2bits(lval)
    return bits

def int2bits(val):
    val = int2uint(val)
    return ushort2bits(val)

def bits2float(bits):
    if(len(bits)==5):
        val = 0
        for i in range(5):
            val+=bits[i]<<(i*7)
        v = [val&0xff,(val>>8)&0xff,(val>>16)&0xff,(val>>24)&0xff]
        return struct.unpack('<f', struct.pack('4B', *v))[0]
    return 0

def bits2short(bits):
    if(len(bits)==2):
        val = 0
        for i in range(2):
            val+=bits[i]<<(i*7)
        return val
    return 0

def bits2long(bits):
    if(len(bits)==5):
        val = 0
        for i in range(5):
            val+=bits[i]<<(i*7)
        return val
    return 0

def int2uint(val):
    if val<0:
        return val+256
    return val

def bits2int8(bits):
    val = 0
    for i in range(2):
        val+=bits[i]<<(i*7)
    if val>127:
        return val-256
    return val

def bytes2long(buf,position=0):
    v = [buf[position], buf[position+1], buf[position+2], buf[position+3]]
    return struct.unpack('<l', struct.pack('4B', *v))[0]

def bytes2short(buf, position=0):
    v = [buf[position], buf[position+1]]
    return struct.unpack('<h', struct.pack('2B', *v))[0]
    
def bytes2float(buf, position=0):
    v = [buf[position], buf[position+1],buf[position+2],buf[position+3]]
    return struct.unpack('<f', struct.pack('4B', *v))[0]

def float2bytes(fval):
    val = struct.pack("f",fval)
    return [val[0],val[1],val[2],val[3]]

def long2bytes(lval):
    val = struct.pack("l",lval)
    return [val[0],val[1],val[2],val[3]]

def short2bytes(sval):
    val = struct.pack("h",sval)
    return [val[0],val[1]]

def ushort2bytes(sval):
    val = struct.pack("H",sval)
    return [val[0],val[1]]

def string2bytes(ssval):
    return [ord(i) for i in ssval]

def bytes2string(buf):
    return "".join([ chr(i) for i in buf])