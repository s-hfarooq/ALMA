import os
import pigpio

pi = pigpio.pi()


def setNewCol(first, second, third):
    pi.set_PWM_dutycycle(PIN_1, first)
    pi.set_PWM_dutycycle(PIN_2, second)
    pi.set_PWM_dutycycle(PIN_3, third)
