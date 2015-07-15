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
        self.refresh_menu()

    def inkey(self, k):
        if self.child == None:
            res = self.inkey_myself(k)
        else:
            res = self.child.inkey(k)

        if res == False:
            self.end_func()
            return False

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

class menu_delay(menu):
    def postinit(self):
        self.menu.append(("ETH0DELAY", tMenu, delay_eth0))
        self.menu.append(("ETH0LOSS" , tMenu, delay_eth0))
        self.menu.append(("ETH1DELAY", tMenu, delay_eth1))
        self.menu.append(("ETH1LOSS" , tMenu, delay_eth1))
        self.menu.append(("SAVE", tFunc, self.save))
        self.refresh_menu()


    def save(self):
        print "commit"
        return(False)

def is_num(k):
    for i in range(10):
        if k == str(i):
            return(True)
    return(False)

class delay_eth0(menu):
    def postinit(self):
        global line1
        line1 = "SETDELAYETH0"
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

class delay_eth1():
    pass

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
