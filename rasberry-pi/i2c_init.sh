#!/bin/sh -x

# rasberry pi script
# read temp from ADT7410
# output to acm1602ni

tempres=`i2cget -y 1 0x48 0 w`
tmp1=`echo $tempres | cut -c 3,4`
tmp2=`echo $tempres | cut -c 5,6`
temp16=`echo $tmp2$tmp1 | tr '[:lower:]' '[:upper:]' `
temp10=`echo "obase=10; ibase=16; $temp16" | bc`
realtemp10=`echo "$temp10 * 0.00625 " | bc | cut  -c 1-5 `
echo $tempres
echo $temp16
echo $realtemp10

outstr=`echo $realtemp10 | sed -e 's/\([0-9]\)/0x3\1 /g' -e 's/\./0x2e /'`
echo $outstr

# Clear Display: 画面の初期化
i2cset -y 1 0x50 0x00 0x01 b
# 列等の設定: 8bit コード、2列, 5x8表示
i2cset -y 1 0x50 0x00 0x38 b
# カーソルは表示、ただし、部リンクしない(Display)
i2cset -y 1 0x50 0x00 0x0e b
i2cset -y 1 0x50 0x00 0x06 b
for s in $outstr; do
 i2cset -y 1 0x50 0x80 $s b
done
i2cset -y 1 0x50 0x80 0xdf b
i2cset -y 1 0x50 0x80 0x43 b

