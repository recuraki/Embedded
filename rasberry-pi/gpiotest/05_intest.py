#!/usr/bin/python
# coding: utf-8
 
import time
import RPi.GPIO as gpio
 
gpio.setmode(gpio.BCM)

# 2をOUT
gpio.setup(2, gpio.OUT)
# 5をINにしてプルダウン(なにも入力がないとダウン＝0)
gpio.setup(5, gpio.IN, pull_up_down=gpio.PUD_DOWN)

while True:
  time.sleep(1)
  if gpio.input(5) == True:
    print "on"
    gpio.output(2, 1)
  else:
    print "off"
    gpio.output(2, 0)
 
