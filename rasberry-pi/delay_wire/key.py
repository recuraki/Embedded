#!/usr/bin/python
# -*- coding: utf-8 -*-


import RPi.GPIO as GPIO
import time

PIN_ROW = [22, 24, 23, 9]
PIN_COL = [10, 17, 11]

def init_pin():
    GPIO.setmode(GPIO.BCM)
    for inPin in PIN_ROW:
        GPIO.setup(inPin, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
    for inPin in PIN_COL:
        GPIO.setup(inPin, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

def key_scan_raw():
    liRes = []
    liRow = []
    liCol = []

    # Col側を読み込む
    for inPin in PIN_ROW:
        GPIO.setup(inPin, GPIO.OUT, initial=GPIO.LOW)
        GPIO.output(inPin, True)

    for inPin in PIN_COL:
        liCol.append(GPIO.input(inPin))

    for inPin in PIN_ROW:
        GPIO.output(inPin, False)
        GPIO.setup(inPin, GPIO.IN,  pull_up_down=GPIO.PUD_DOWN)

    # Row側を読み込む
    for inPin in PIN_COL:
        GPIO.setup(inPin, GPIO.OUT, initial=GPIO.LOW)
        GPIO.output(inPin, True)

    for inPin in PIN_ROW:
        liRow.append(GPIO.input(inPin))

    for inPin in PIN_COL:
        GPIO.output(inPin, False)
        GPIO.setup(inPin, GPIO.IN,  pull_up_down=GPIO.PUD_DOWN)

    return(liCol,liRow)

def key_scan():
    table = []
    table.append( ["1", "2", "3"])
    table.append( ["4", "5", "6"])
    table.append( ["7", "8", "9"])
    table.append( ["*", "0", "#"])

    c, r = key_scan_raw()

    for i in [0, 1, 2]:
        if c[i] == True:
            for j in [0, 1, 2, 3]:
                if r[j] == True:
                    return table[j][i]
    return ""


if __name__ == "__main__":
    GPIO.cleanup()
    init_pin()
    print key_scan()
