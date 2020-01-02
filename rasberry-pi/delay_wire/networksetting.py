#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import os
import re

templ="""
auto lo 
auto br0
iface lo inet loopback

iface eth0 inet manual
iface eth1 inet manual
iface br0 inet static
  bridge_ports eth0 eth1
  address {addr}
  netmask {netmask}
  gateway {gw}
"""

class setting():
    fn = "/etc/network/interfaces"
    addr, netmask, gw = "", "", ""
    def __init__(self, fn = None):
        if fn:
            self.fn = fn
        fd = open(self.fn)
        self.dat = fd.read()
        fd.close()

    def load(self):
        print self.dat
        self.addr =    re.search("address ([0-9\.]*)\n", self.dat).group(1)
        self.netmask = re.search("netmask ([0-9\.]*)\n", self.dat).group(1)
        self.gw =      re.search("gateway ([0-9\.]*)\n", self.dat).group(1)

    def save(self):
        d = {}
        d["addr"] = self.addr
        d["netmask"] = self.netmask
        d["gw"] = self.gw
        with open(self.fn, "w") as fd:
            fd.write(templ.format(**d))
        
    def setaddr(self, str):
        self.addr = str

    def setnetmask(self, str):
        self.netmask = str

    def setgw(self, str):
        self.gw = str
        

if __name__ == "__main__":
    d = {}
    d["addr"] = "hoge"
    d["netmask"] = "m"
    d["gw"] = "a"
    print(templ.format(**d))
    s = setting()
    s.load()
    s.setaddr("192.168.100.200")
    s.save()
