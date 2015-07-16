#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import termios
import tty

def key_scan():
    i = sys.stdin.fileno()
    oattr = termios.tcgetattr(i)
    tty.setraw(i)
    c = sys.stdin.read(1)
    termios.tcsetattr(i, termios.TCSADRAIN, oattr)
    return(c)

line1 = ""
line2 = ""

def line_out():
    print line1
    print line2

tMenu=0
tFunc=1
eth0delay = 0
eth0loss = 0
eth1delay = 0
eth1loss = 0

ipaddr = "172.16.0.1"
netmask = "255.255.255.255"
defaultgw = "172.16.0.2"

settcCmd = "tc chnage qdisc change dev {eth} root delay {delay}ms loss {loss}%"

def run_settc():
    global eth0delay, eth0loss
    global eth1delay, eth1loss
    stCmd = "tc"
    # eth0
    diArg = {}
    diArg["eth"] = "eth0"
    diArg["delay"] = eth0delay
    diArg["loss"] = eth0loss
    stCmd = settcCmd.format(**diArg)
    run_cmd(stCmd)
    # eth1
    diArg = {}
    diArg["eth"] = "eth1"
    diArg["delay"] = eth1delay
    diArg["loss"] = eth1loss
    stCmd = settcCmd.format(**diArg)
    run_cmd(stCmd)

setgwCmd = "ip route change default via {gwaddr} dev {eth}"
setaddrCmd = "ip addr replace {ipaddr}/{netmask} dev {eth}"
def run_setnetworks():
    pass

def run_cmd(stCmd):
    print stCmd

class menu(object):

    def __init__(self):
        self.mode = 0
        self.pos = 0
        self.child = None
        self.menu = []
        self.postinit()

    def postinit(self):
        self.menu.append(("DELAY", tMenu, menu_delay))
        self.menu.append(("IPADDRESS", tMenu, menu_delay))
        self.menu.append(("DUMP", tFunc, self.dump))
        self.refresh_menu()

    def inkey(self, k):
        if self.child == None:
            res = self.inkey_myself(k)
            if res == False:
                self.end_func()
                return False
        else:
            res = self.child.inkey(k)
            if res == False:
                self.end_func()

        return True

    def end_func(self):
        self.mode = 0
        self.child = None
        self.refresh_menu()

    def inkey_myself(self, k):
        if k == "8":
            if self.pos != 0:
                self.pos -= 1
                self.refresh_menu()
                return(True)
        elif k == "2":
            if self.pos != (len(self.menu) - 1):
                self.pos += 1
                self.refresh_menu()
                return(True)
        elif k == "#":
            return(self.run_pos())
        elif k == "*":
            return(False)

    def refresh_menu(self):
        global line1
        line1 = self.menu[self.pos][0]
        line_out()
        return True

    def run_pos(self):
        if self.menu[self.pos][1] == tMenu:
            self.mode = 1
            self.child = self.menu[self.pos][2]()
            return(True)
        if self.menu[self.pos][1] == tFunc:
            return(self.menu[self.pos][2]())

    def dump(self):
        global eth0delay, eth0loss
        global eth1delay, eth1loss
        print "DUMP START"
        print "eth0delay: " + str(eth0delay) + "ms"
        print "eth0loss: " + str(eth0loss) + "%"
        print "eth1delay: " + str(eth1delay) + "ms"
        print "eth1loss: " + str(eth1loss) + "%"
        run_settc()
        print "DUMP END"
        return(False)


class menu_delay(menu):
    def postinit(self):
        self.menu.append(("ETH0DELAY", tMenu, delay_eth0))
        self.menu.append(("ETH0LOSS" , tMenu, loss_eth0))
        self.menu.append(("ETH1DELAY", tMenu, delay_eth1))
        self.menu.append(("ETH1LOSS" , tMenu, loss_eth1))
        self.refresh_menu()

def is_num(k):
    for i in range(10):
        if k == str(i):
            return(True)
    return(False)



class menu_num(menu):
    pos = 0
    mode = 0
    addr = ""
    target = ""
    maxlen = 4
    suffix = ""

    def postinit(self):
        pass

    def inkey(self, k):
        if self.mode == 0:
            if is_num(k):
                self.addr += k
                self.pos += 1
            if self.pos == self.maxlen:
                print self.addr
                self.mode = 1
        if k == "#":
            return(self.save())
        elif k == "*":
            return(False)
        self.refresh_out()

    def save(self):
        stMs = "".join(self.addr)
        if stMs == "":
            stMs = "0"
        ms = int(stMs)
        self.save_main(ms)
        return(False)

    def save_main(self, ms):
        pass

    def refresh_out(self):
        global line2
        addrstr = "".join(self.addr) + self.suffix
        line2 = addrstr
        line_out()

class delay_eth0(menu_num):
    def postinit(self):
        global line1
        self.target = "eth0"
        line1 = "SETDELAYETH0"
        self.suffix = "ms"
        line_out()

    def save_main(self, ms):
        global eth0delay
        print "commit: eth0delay:" + str(ms)
        eth0delay = ms
        run_settc()
        return(True)

class delay_eth1(menu_num):
    def postinit(self):
        global line1
        self.target = "eth1"
        line1 = "SETDELAYETH1"
        self.suffix = "ms"
        line_out()

    def save_main(self, ms):
        global eth1delay
        print "commit: eth1delay:" + str(ms)
        eth1delay = ms
        run_settc()
        return(True)

class loss_eth0(menu_num):
    def postinit(self):
        global line1
        self.target = "eth0"
        line1 = "SETLOSSETH0"
        self.suffix = "%"
        self.maxlen = 3
        line_out()

    def save_main(self, rate):
        global eth0loss
        if rate > 100:
            rate = 100
        print "commit: eth0loss:" + str(rate)
        eth0loss = rate
        run_settc()
        return(True)

class loss_eth1(menu_num):
    def postinit(self):
        global line1
        self.target = "eth1"
        line1 = "SETLOSSETH1"
        self.suffix = "%"
        self.maxlen = 3
        line_out()

    def save_main(self, rate):
        global eth1loss
        if rate > 100:
            rate = 100
        print "commit: eth1loss:" + str(rate)
        eth1loss = rate
        run_settc()
        return(True)

class set_ipaddr(menu):
    def postinit(self):
        global line1
        line1 = "BR0ADDR"
        self.pos = 0
        self.addr = [""] * 12
        line_out()

    def inkey(self, k):
        if is_num(k):
            self.addr[self.pos] = k
            self.pos += 1
        if self.pos == 12:
            print self.addr
        self.refresh_ipaddr()

    def refresh_ipaddr(self):
        global line2
        addrstr = ".".join(map(lambda x: "".join(x), zip(*[iter(self.addr)]*3)))
        line2 = addrstr
        line_out()

if __name__ == "__main__":
    isLoop = True
    m = menu()
    while isLoop:
        c = key_scan()
        if c == ".":
            isLoop = False
        m.inkey(c)

