

import socket
import sys


IP_ADDRESS = "192.168.0.237"
IP_PORT = 10000

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Connect the socket to the port where the server is listening
server_address = (IP_ADDRESS, IP_PORT)
print('connecting to {} port {}'.format(*server_address))
sock.connect(server_address)

try:

    while True:
        # Send data
        #message = b'This is the message.  It will be repeated.'
        message = input("message: ").encode()
        print('sending {!r}'.format(message))
        sock.sendall(message)

        # Look for the response
        # amount_received = 0
        # amount_expected = len(message)
        #
        # while amount_received < amount_expected:
        data = sock.recv(16)
        #amount_received += len(data)
        print('received {!r}'.format(data))

finally:
    print('closing socket')
    sock.close()
