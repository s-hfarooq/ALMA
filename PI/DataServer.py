import socket
import sys
import RPi.GPIO as GPIO

IP_ADDRESS = "192.168.0.237"
IP_PORT = 10000
PIN = 8

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Bind the socket to the port
server_address = (IP_ADDRESS, IP_PORT)
print('starting up on {} port {}'.format(*server_address))
sock.bind(server_address)

# Listen for incoming connections
sock.listen(1)

GPIO.setmode(GPIO.BOARD)
GPIO.setup(PIN, GPIO.OUT)


currentState = 'OFF'

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
                        GPIO.output(PIN, 1)
                        currentState = 'ON'
                        connection.sendall(b"Turned to high")
                    else:
                        connection.sendall(b"Already high")
                elif data == b'off':
                    if currentState != 'OFF':
                        GPIO.output(PIN, 0)
                        currentState = 'OFF'
                        connection.sendall(b"Turned to low")
                    else:
                        connection.sendall(b"Already low")
                elif data == b'fade':
                    if currentState != 'FADE':
                        # call FADE
                        currentState = 'FADE'
                        connection.sendall(b"Turned to fade (not currently implemented)")
                    else:
                        connection.sendall(b"Already fade")
                else:
                    connection.sendall(b"Unrecognized command")

            else:
                print('no data from', client_address)
                break

    finally:
        # Clean up the connection
        connection.close()
