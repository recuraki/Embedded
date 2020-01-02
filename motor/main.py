#!/usr/bin/env python
# -*- coding: utf-8 -*-

import smbus
import time
import collections
from optparse import OptionParser
import sys
import pathlib
import pandas as pd
import pickle
import sklearn
import numpy as np

clf = pickle.load(open("finalized.pickle", 'rb'))

basepath="./datas/{0}"

bus = smbus.SMBus(1)
bus.write_i2c_block_data(0x68, 0b10010011, [0x00])
time.sleep(1)

time_limit_recode = 5 # sec
time_min_recode=1 # sec この時間以上は記録しないと保存しない
time_end_interval = 0.5 # sec  この時間以上空白が続くとその回は終わりと判定する
def proc_recode():
 datas = collections.deque([])
 state = 0
 time_thiszero = time.time()
 time_lastinput = 0
 time_start_recode = 0
 # 0:init, 1:recoding
 stat = 0
 is_last_zero = True

 while True:

  data = bus.read_i2c_block_data(0x68, 0b10010011, 2)
  raw  = data[0] << 8 | data[1]
  if raw > 32767:
   raw -= 65535
  vol = 2.048 / 2047
  value = raw * vol
  t, v = time.time(),value
  if abs(value < 0.01):
    value = 0

  if stat == 1 and (t - time_start_recode) > time_limit_recode:
      print("Recode: Error: too long")
      return None

  if value == 0:
      if is_last_zero is False:
          is_last_zero = True
          time_thiszero = t
      if stat == 1:
          datas.append((t,v))

      if stat == 1 and (t - time_thiszero) >  time_end_interval:
          if (time_thiszero - time_start_recode) < time_min_recode:
              print("Recode: Error: too short")
              return None
          print("Recode: Finish ok")
          return datas
  else:
      is_last_zero = False
      time_lastinput = t
      datas.append((t,v))
      if stat == 0:
          if len(datas) > 0 and (t - datas[0][0] > time_limit_recode):
              datas.popleft()
          print("Recode: Start()")
          time_start_recode = t
          stat = 1
      
  time.sleep(0.005)

def do(df):
    v = df.iloc[0:, 1].values.flatten()
    v = v
    dt = df["time"].diff().mean()
    fs = 1/dt
    N = len(v)
    fftX = np.fft.fft(v)
    freqList = np.fft.fftfreq(N, d=1.0/fs)
    amplitudeSpectrum =  [np.sqrt(c.real ** 2 + c.imag ** 2) for c in fftX]
    amplitudeSpectrum = amplitudeSpectrum[:80]
    return amplitudeSpectrum

yubiMap = {}
with open("output_attr.csv", "r") as fp:
    for l in fp.readlines():
        t = l.strip().split(",")
        yubiMap[t[0]] = t[1]



count = 0
while True:
 print("Cycle:{0} Start".format(count))
 d = proc_recode()
 if d is None:
     continue
 #pickle.dump(d, open("test.dump", "wb"))
 #d = pickle.load(open("test.dump", "rb"))
 thistime = int(time.time())
 l = len(d)
 for i in range(l - 1, -1, -1):
     if d[i][1] < 0.15:
         d.pop()
         continue
     break

 data = pd.DataFrame(list(d), columns=("time", "volt"))
 res = do(data)
 clfres = clf.predict([res])
 clfres = clfres[0]
 print(clfres)
 print(yubiMap[clfres])
 
 #break
 count += 1

