#!/bin/sh
# 初期設定
echo "2" > /sys/class/gpio/export
echo "out" > /sys/class/gpio/gpio2/direction
# 繰り返し
while true; do
 # LEDをON
 echo "1" > /sys/class/gpio/gpio2/value
 # 1行読み込む
 sleep 1
 # LEDをOFF
 echo "0" > /sys/class/gpio/gpio2/value
 # 1行読み込む
 sleep 1
done

