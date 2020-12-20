import socket
import sys
import subprocess
import os
import time

IP_ADDRESS = "192.168.1.132"
IP_PORT = 8090

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Bind the socket to the port
server_address = (IP_ADDRESS, IP_PORT)
print('starting up on {} port {}'.format(*server_address))
sock.bind(server_address)

# Listen for incoming connections
sock.listen(1)

while True:
    # Wait for a connection
    print('waiting for a connection')
    connection, client_address = sock.accept()
    try:
        print('connection from', client_address)

        # Receive the data in small chunks and retransmit it
        data = connection.recv(1024)
        print('received {!r}'.format(data))

    finally:
        # Clean up the connection
        connection.close()
