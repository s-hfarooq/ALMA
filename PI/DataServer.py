# DataServer2.py

from tcpcom import TCPServer
import time
import RPi.GPIO as GPIO

IP_PORT = 22000
PIN = 8 # adapt to your wiring

def onStateChanged(state, msg):
    if state == "LISTENING":
        print "Server:-- Listening..."
    elif state == "CONNECTED":
        print "Server:-- Connected to", msg
    elif state == "MESSAGE":
        print "Server:-- Message received:", msg
        if msg == "on":
            if GPIO.input(PIN) == GPIO.LOW:
                GPIO.output(PIN, 1)
                server.sendMessage("Turned to high")
            else:
                server.sendMessage("Already high")
        else:
            if GPIO.input(PIN) == GPIO.HIGH:
                GPIO.output(PIN, 0)
                server.sendMessage("Turned to low")
            else:
                server.sendMessage("Already low")
def setup():
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(PIN, GPIO.OUT)

setup()
server = TCPServer(IP_PORT, stateChanged = onStateChanged)
