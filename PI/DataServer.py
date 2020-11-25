import socket
import sys
import subprocess
import RPi.GPIO as GPIO
import os
import pigpio
import time

IP_ADDRESS = "192.168.0.237"
IP_PORT = 10000

PIN_1_GP = 14
PIN_2_GP = 15
PIN_3_GP = 18
PIN_1_SEC = 23
PIN_2_SEC = 24
PIN_3_SEC = 25

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Bind the socket to the port
server_address = (IP_ADDRESS, IP_PORT)
print('starting up on {} port {}'.format(*server_address))
sock.bind(server_address)

# Listen for incoming connections
sock.listen(1)
pi = pigpio.pi()

# Functions to set colors on each strip
def setNewCol(first, second, third):
    pi.set_PWM_dutycycle(PIN_1_GP, first)
    pi.set_PWM_dutycycle(PIN_2_GP, second)
    pi.set_PWM_dutycycle(PIN_3_GP, third)
def setNewColSec(first, second, third):
    pi.set_PWM_dutycycle(PIN_1_SEC, first)
    pi.set_PWM_dutycycle(PIN_2_SEC, second)
    pi.set_PWM_dutycycle(PIN_3_SEC, third)

currentState = 'OFF'
fProc = None

# Initially turn lights on
setNewCol(255, 255, 255)
setNewColSec(255, 255, 255)

while True:
    # Wait for a connection
    print('waiting for a connection')
    connection, client_address = sock.accept()
    try:
        print('connection from', client_address)

        # Receive the data in small chunks and retransmit it
        while True:
            data = connection.recv(16).decode("utf-8")
            print('received {!r}'.format(data))
            if data:
                print("Server:-- Message received:", data)
                if data == 'on':
                    if currentState != 'ON':
                        if currentState == 'FADE':
                            fProc.kill()
                            fProc = None
                        setNewCol(255, 255, 255)
                        setNewColSec(255, 255, 255)
                        currentState = 'ON'
                        connection.sendall(b"Turned to high")
                    else:
                        connection.sendall(b"Already high")
                elif data == 'off':
                    if currentState != 'OFF':
                        if currentState == 'FADE':
                            fProc.kill()
                            fProc = None
                        setNewCol(0, 0, 0)
                        setNewColSec(0, 0, 0)
                        currentState = 'OFF'
                        connection.sendall(b"Turned to low")
                    else:
                        connection.sendall(b"Already low")
                elif data == 'fade':
                    if currentState != 'FADE':
                        fProc = subprocess.Popen([sys.executable, "fading.py"])
                        currentState = 'FADE'
                        connection.sendall(b"Turned to fade")
                    else:
                        connection.sendall(b"Already fade")

                elif "col" in data:
                    vals = [int(i) for i in data.split() if i.isdigit()]
                    if len(vals) < 3:
                        vals = [255, 255, 255]

                    for i in range(0, 3):
                        if vals[i] > 255 or vals[i] < 0:
                            while vals[i] < 0:
                                vals[i] = vals[i] + 255
                            vals[i] = vals[i] % 255

                    if "col2" in data:
                        setNewColSec(vals[0], vals[1], vals[2])
                    elif "colB" in data:
                        setNewCol(vals[0], vals[1], vals[2])
                        setNewColSec(vals[0], vals[1], vals[2])
                    else:
                        setNewCol(vals[0], vals[1], vals[2])

                    currentState = 'COL'
                    connection.sendall(b"Set new col")
                else:
                    connection.sendall(b"not recognized")

            else:
                print('no data from', client_address)

    finally:
        # Clean up the connection
        connection.close()
