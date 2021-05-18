# pi master, stm slave
import time
from smbus import SMBus
from struct import *

addr = 0x0F # slave address
bus = SMBus(1)

def read_data():
    read = 0
    while read == 0:
        try:
            print("reading data")
            block = bus.read_i2c_block_data(addr, 0, 28)
            bytes_of_values = bytes(block)
            data = unpack('>iiiiiii', bytes_of_values)
            #print(bytes_of_values)
            #print(data)
            #print("block" + str(block))
            return data
            
        except Exception as e:
            print("read error" + str(e))
            continue
        read = 1

def cal_distance(raw_data):
    # cm
    distance = []
    for data in raw_data:
        if data != -1:
            distance.append(data/58)
        else:
            distance.append(-1)
    return distance
