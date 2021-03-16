# # -*- coding: utf-8 -*-
# import smbus
# import time
# import sys
# # for RPI version 1, use bus = smbus.SMBus(0)
# bus = smbus.SMBus(1)
#
# # This is the address we setup in the Arduino Program
# address = 0x04
#
# #http://www.raspberry-projects.com/pi/programming-in-python/i2c-programming-in-python/using-the-i2c-interface-2
# def writeData(value):
#     byteValue = StringToBytes(value)
#     bus.write_i2c_block_data(address,0x00,byteValue) #first byte is 0=command byte.. just is.
#     return -1
#
#
# def StringToBytes(val):
#         retVal = []
#         for c in val:
#                 retVal.append(ord(c))
#         return retVal
#
# args = list(sys.argv)
# print('Sending...')
# writeData("-" + args[1])



# -*- coding: utf-8 -*-
import smbus
import time
import sys
import io
# for RPI version 1, use bus = smbus.SMBus(0)
bus = smbus.SMBus(1)

# This is the address we setup in the Arduino Program
address = 0x04

#http://www.raspberry-projects.com/pi/programming-in-python/i2c-programming-in-python/using-the-i2c-interface-2
def writeData(value):
    byteValue = StringToBytes(value)
    bus.write_i2c_block_data(address,0x00,byteValue) #first byte is 0=command byte.. just is.
    return -1

def StringToBytes(val):
        retVal = []
        for c in val:
                retVal.append(ord(c))
        return retVal


while(True):
    #args = list(sys.argv)
    txt = input("Cmd: ")
    # strings = ['255-0-0-0-0-0-', '0-255-0-0-0-0-', '0-0-255-0-0-0-', '0-0-0-0-0-0-']
    # for i in range(len(strings)):
    #     print('Sending ', strings[i])
    #     writeData("-" + strings[i])
    #     time.sleep(0.5)

    writeData("-" + txt)
