#!/usr/bin/env python

import io
import fcntl
import sys

# i2c_raw.py
# 2016-02-26
# Public Domain

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

    # Send data over i2c to ESP32
    while(True):
        dev = i2c_raw.i2c(0x04, 1) # device 0x04, bus 1
        txt = input("Cmd: ")
        strLen = chr(len(txt))
        dev.write((strLen + txt).encode('utf-8'))
        dev.close()
