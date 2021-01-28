import socket
import sys

IP_ADDRESS = "192.168.0.114"
IP_PORT = 3333

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Connect the socket to the port where the server is listening
server_address = (IP_ADDRESS, IP_PORT)
print('connecting to {} port {}'.format(*server_address))
sock.connect(server_address)

try:

    while True:
        # Send data
        message = (input("message: ")).encode()
        print('sending {!r}'.format(message))
        sock.sendall(message)

        data = sock.recv(1024)
        print('received {!r}'.format(data))

finally:
    print('closing socket')
    sock.close()
