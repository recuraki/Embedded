#!/usr/bin/python
# coding: utf-8
 
import time
# RaspberryPIに標準で入っている専用ライブラリ
# RPi.GPIOをgpioとしてload
import RPi.GPIO as gpio
 
# gpioの選択をピン番号で行う(おまじないだと思ってください)
gpio.setmode(gpio.BCM)
 
# GPIO2をOUTに使う
gpio.setup(2, gpio.OUT)

while True:
  gpio.output(2, 1)
  time.sleep(1)
  gpio.output(2, 0)
  time.sleep(1)
