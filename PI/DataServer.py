import socket
import sys
import subprocess
import RPi.GPIO as GPIO
import os
import pigpio

IP_ADDRESS = "192.168.0.237"
IP_PORT = 10000
PIN_1 = 8
PIN_2 = 10
PIN_3 = 12

PIN_1_GP   = 14
PIN_2_GP = 15
PIN_3_GP  = 18

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Bind the socket to the port
server_address = (IP_ADDRESS, IP_PORT)
print('starting up on {} port {}'.format(*server_address))
sock.bind(server_address)

# Listen for incoming connections
sock.listen(1)

GPIO.setmode(GPIO.BOARD)
GPIO.setup(PIN_1, GPIO.OUT)
GPIO.setup(PIN_2, GPIO.OUT)
GPIO.setup(PIN_3, GPIO.OUT)


pi = pigpio.pi()

def setNewCol(first, second, third):
    pi.set_PWM_dutycycle(PIN_1_GP, first)
    pi.set_PWM_dutycycle(PIN_2_GP, second)
    pi.set_PWM_dutycycle(PIN_3_GP, third)


currentState = 'OFF'
fProc = None

while True:
    # Wait for a connection
    print('waiting for a connection')
    connection, client_address = sock.accept()
    try:
        print('connection from', client_address)

        # Receive the data in small chunks and retransmit it
        while True:
            data = connection.recv(16)
            print('received {!r}'.format(data))
            if data:
                # print('sending data back to the client')
                # connection.sendall(data)

                print("Server:-- Message received:", data)
                if data == b'on':
                    if currentState != 'ON':
                        if currentState == 'FADE':
                            fProc.kill()
                            fProc = None
                        GPIO.output(PIN_1, 1)
                        GPIO.output(PIN_2, 1)
                        GPIO.output(PIN_3, 1)
                        currentState = 'ON'
                        connection.sendall(b"Turned to high")
                    else:
                        connection.sendall(b"Already high")
                elif data == b'off':
                    if currentState != 'OFF':
                        if currentState == 'FADE':
                            fProc.kill()
                            fProc = None
                        GPIO.output(PIN_1, 0)
                        GPIO.output(PIN_2, 0)
                        GPIO.output(PIN_3, 0)
                        currentState = 'OFF'
                        connection.sendall(b"Turned to low")
                    else:
                        connection.sendall(b"Already low")
                elif data == b'fade':
                    if currentState != 'FADE':
                        fProc = subprocess.Popen([sys.executable, "fading.py"])
                        currentState = 'FADE'
                        connection.sendall(b"Turned to fade")
                    else:
                        connection.sendall(b"Already fade")

                elif data == b'test':
                    setNewCol(232, 74, 39)
                    currentState = 'TEST'
                    connection.sendall(b"Turned to test color")
                else:
                    connection.sendall(b"not recognized")

            else:
                print('no data from', client_address)
                break

    finally:
        # Clean up the connection
        connection.close()
