#!/usr/bin/env python

import io
import fcntl
import sys
import random

# i2c_raw.py
# 2016-02-26
# Public Domain

availStrings = ["{\"senderUID\": \"10000123\", \"receiverUID\": \"101FFFFF\", \"functionID\": \"15\", \"data\": [255, 0, 12]}",
"{\"senderUID\": \"10000123\", \"receiverUID\": \"101FFFFF\", \"functionID\": \"12\", \"data\": [255, 0, 12]}",
"{\"senderUID\": \"10000123\", \"receiverUID\": \"101FFFFF\", \"functionID\": \"-1\", \"data\": [255, 0, 12]}",
"{\"senderUID\": \"10000123\", \"receiverUID\": \"102FFFFF\", \"functionID\": \"0\", \"data\": [0, 0, 255]}",
"{\"senderUID\": \"10000123\", \"receiverUID\": \"102FFFFF\", \"functionID\": \"0\", \"data\": [255, 255, 255]}",
"{\"senderUID\": \"10000123\", \"receiverUID\": \"102FFFFF\", \"functionID\": \"3\", \"data\": [25]}"]


availStringsType = ["Holonyak theaterChase",
"Holonyak runningLights",
"Holonyak color 255, 0, 12",
"5050 blue",
"5050 white",
"5050 fade"]

I2C_SLAVE=0x0703

if sys.hexversion < 0x03000000:
    def _b(x):
        return x
else:
    def _b(x):
        return x.encode('latin-1')

class i2c:
    # Open device, set address
    def __init__(self, device, bus):
        self.fr = io.open("/dev/i2c-"+str(bus), "rb", buffering=0)
        self.fw = io.open("/dev/i2c-"+str(bus), "wb", buffering=0)

        # Set device address
        fcntl.ioctl(self.fr, I2C_SLAVE, device)
        fcntl.ioctl(self.fw, I2C_SLAVE, device)

    # Write to i2c
    def write(self, data):
        print(type(data))
        if type(data) is list:
            data = bytearray(data)
        elif type(data) is str:
            data = _b(data)
        self.fw.write(data)

    # Read i2c
    def read(self, count):
        return self.fr.read(count)

    # Close connection
    def close(self):
        self.fw.close()
        self.fr.close()


if __name__ == "__main__":
    import time
    import i2c_raw
    import sys

    i = 0

    # Send data over i2c to ESP32
    while(True):
        dev = i2c_raw.i2c(0x04, 1) # device 0x04, bus 1
        # txt = input("Cmd: ")
        idx = random.randint(0, len(availStrings) - 1)
        txt = availStrings[idx]
        print("Sending command:", availStringsType[idx], "(i:", i, ")")
        strLen = chr(len(txt))
        dev.write((strLen + txt).encode('utf-8'))
        dev.close()
        i += 1
        time.sleep(2.5)