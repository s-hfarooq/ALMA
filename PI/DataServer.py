# DataServer2.py

from tcpcom import TCPServer
import time
import RPi.GPIO as GPIO

IP_PORT = 22000
P_BUTTON = 8 # adapt to your wiring

def onStateChanged(state, msg):
    if state == "LISTENING":
        print "Server:-- Listening..."
    elif state == "CONNECTED":
        print "Server:-- Connected to", msg
    elif state == "MESSAGE":
        print "Server:-- Message received:", msg
        if msg == "go":
            if GPIO.input(P_BUTTON) == GPIO.LOW:
                server.sendMessage("Button pressed")
            else:
                server.sendMessage("Button released")

def setup():
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(P_BUTTON, GPIO.IN, GPIO.PUD_UP)

setup()
server = TCPServer(IP_PORT, stateChanged = onStateChanged)
