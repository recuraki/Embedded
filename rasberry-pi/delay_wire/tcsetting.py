#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import os
import re
import subprocess

settcCmd = "sudo tc qdisc add dev {eth} root netem delay {delay}ms loss {loss}%"
deltcCmd = "sudo tc qdisc delete dev {eth} root"
showtcCmd = "sudo tc qdisc show"


class setting():
    eth0delay = 0
    eth0loss = 0
    eth1delay = 0
    eth1loss = 0
    
    def __init__(self):
        pass

    def load(self):
        res = subprocess.check_output(showtcCmd.split(" "))

        a = re.search("qdisc netem.*dev eth0.*delay (\d*).0ms", res)
        if a != None:
            self.eth0delay = a.group(1)

        a = re.search("qdisc netem.*dev eth0.*loss (\d*)%", res)
        if a != None:
            self.eth0loss = a.group(1)

        a = re.search("qdisc netem.*dev eth1.*delay (\d*).0ms", res)
        if a != None:
            self.eth1delay = a.group(1)

        a = re.search("qdisc netem.*dev eth1.*loss (\d*)%", res)
        if a != None:
            self.eth1loss = a.group(1)

    def save(self):
        subprocess.call(deltcCmd.format(eth="eth0").split(" "))
        subprocess.call(deltcCmd.format(eth="eth1").split(" "))
        subprocess.call(settcCmd.format(eth="eth0", delay=self.eth0delay, loss=self.eth0loss).split(" "))
        subprocess.call(settcCmd.format(eth="eth1", delay=self.eth1delay, loss=self.eth1loss).split(" "))
        print settcCmd.format(eth="eth0", delay=self.eth0delay, loss=self.eth0loss)
        print settcCmd.format(eth="eth1", delay=self.eth1delay, loss=self.eth1loss)

    def dump(self):
        print "e0d, e0l, e1d, e1l = {0}, {1}, {2}, {3}".format(self.eth0delay, self.eth0loss, self.eth1delay, self.eth1loss) 

if __name__ == "__main__":
    s = setting()
    s.load()
    s.dump()
    s.save()
    s.load()            
