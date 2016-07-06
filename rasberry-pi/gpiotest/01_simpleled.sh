#!/bin/sh
# 初期設定(以下をコピペ)
echo "2" > /sys/class/gpio/export
echo "3" > /sys/class/gpio/export
echo "4" > /sys/class/gpio/export
# 以下を入力するとLEDが消えるはず
echo "out" > /sys/class/gpio/gpio2/direction
echo "out" > /sys/class/gpio/gpio3/direction
echo "out" > /sys/class/gpio/gpio4/direction

# 以下は手で繰り返し打ってみてください
# ON
echo "1" > /sys/class/gpio/gpio2/value
# OFF
echo "0" > /sys/class/gpio/gpio2/value

