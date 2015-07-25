#!/usr/bin/python
# -*- coding: utf-8 -*-

# Python2はipaddrモジュールを持っていない
# sudo pip install py2-ipaddress

import subprocess
import time
import sys
import termios
import tty
import ipaddress
import key
import networksetting 
import tcsetting
import optparse
from acm1602 import acm1602

netset = networksetting.setting()
netset.load()
ipaddr = netset.addr
netmask = netset.netmask
defaultgw = netset.gw

tcset = tcsetting.setting()
tcset.load()
eth0delay = tcset.eth0delay
eth0loss  = tcset.eth0loss
eth1delay = tcset.eth1delay
eth1loss  = tcset.eth1loss

line1 = ""
line2 = ""
tMenu=0
tFunc=1

settcCmd = "tc chnage qdisc change dev {eth} root delay {delay}ms loss {loss}%"
setgwCmd = "ip route change default via {gwaddr} dev {eth}"
setaddrCmd = "ip addr replace {addrstr} dev {eth}"

def key_scan():
    """
    1文字だけget charしてくる
    termIOsでがんばる
    """
    i = sys.stdin.fileno()
    oattr = termios.tcgetattr(i)
    tty.setraw(i)
    c = sys.stdin.read(1)
    termios.tcsetattr(i, termios.TCSADRAIN, oattr)
    return(c)

def line_out():
    print line1
    print line2

def run_settc():
    """
    TCコマンドの実行
    """
    global eth0delay, eth0loss
    global eth1delay, eth1loss
    tcset.eth0delay = eth0delay
    tcset.eth0loss = eth0loss
    tcset.eth1delay = eth1delay
    tcset.eth1loss = eth1loss
    tcset.save()

def run_setnetworks():
    """
    ネットワーク系コマンドの実行
    """
    global ipaddr, netmask, defaultgw
    diArg = {}
    diArg["eth"] = "br0"
    diArg["gwaddr"] = defaultgw
    addr = ipaddress.IPv4Network("0.0.0.0" + "/" + netmask)
    masknum = str(addr).split("/")[1]
    diArg["addrstr"] = ipaddr + "/" + masknum
    stCmd = setaddrCmd.format(**diArg)
    run_cmd(stCmd)
    stCmd = setgwCmd.format(**diArg)
    run_cmd(stCmd)
    # /etc/networks/interfacesに書く
    netset.addr = ipaddr
    netset.netmask = netmask
    netset.gw = defaultgw
    netset.save()

def run_cmd(stCmd):
    print stCmd
    subprocess.call(stCmd.split(" "))

class menu(object):

    def __init__(self):
        self.mode = 0
        self.pos = 0
        self.child = None
        self.menu = []
        self.postinit()

    def postinit(self):
        """
        主にメニューの追加
        """
        self.menu.append(("DELAY>>", tMenu, menu_delay, "SETTTING>"))
        self.menu.append(("NETWORK>>", tMenu, menu_addr, " SETTING>"))
        self.menu.append(("DUMP", tFunc, self.dump, ""))
        self.menu.append(("REBOOT>>", tMenu, menu_reboot, ""))
        self.refresh_menu()

    def inkey(self, k):
        global line1
        global line2

        """
        キー入力を受け取った時の処理
        """

        # 自分の子供がいないときは自分自身の処理部分にキー入力を渡す
        if self.child == None:
            res = self.inkey_myself(k)
            # Falseが返ってきた場合、自分自身を終了する
            # = 親にFalseを返す
            if res == False:
                line2 = ""
                self.end_func()
                return False
        # 子供がいる場合、子どもに入力として渡す
        else:
            res = self.child.inkey(k)
            # Falseが子供から帰ってきた場合は
            # 子を消す
            if res == False:
                line2 = ""
                self.end_func()

        return True

    def end_func(self):
        self.mode = 0
        self.child = None
        self.menu = []
        self.postinit()

    def inkey_myself(self, k):
        # 自分自身がキー入力を受け取った時

        # 2,8の場合、適宜、メニューを上下する
        if k == "2":
            if self.pos != 0:
                self.pos -= 1
                self.refresh_menu()
                return(True)
        elif k == "8":
            if self.pos != (len(self.menu) - 1):
                self.pos += 1
                self.refresh_menu()
                return(True)
        # #はコマンドの実行
        elif k == "#":
            return(self.run_pos())
        # *はキャンセル = 上に戻る
        elif k == "*":
            return(False)

    def refresh_menu(self):
        # メニューを更新する
        global line1, line2
        line1 = self.menu[self.pos][0]
        line2 = self.menu[self.pos][3]
        line_out()
        return True
    
    def run_pos(self):
        # コマンドの実行

        # 配下がメニューである場合、それを子供として自分に登録する
        if self.menu[self.pos][1] == tMenu:
            self.mode = 1
            self.child = self.menu[self.pos][2]()
            return(True)

        # それがコマンドである場合は自分自身のそれを実行する
        if self.menu[self.pos][1] == tFunc:
            return(self.menu[self.pos][2]())

    def dump(self):
        # メインメニューのDUMP
        # これはデバッグ用
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
    # DELAY設定用
    def postinit(self):
        global eth0delay, eth1delay
        global eth0loss, eth1loss
        self.menu.append(("ETH0DELAY>>", tMenu, delay_eth0, str(eth0delay) + "ms"))
        self.menu.append(("ETH0LOSS>>" , tMenu, loss_eth0,  str(eth0loss) + "%"))
        self.menu.append(("ETH1DELAY>>", tMenu, delay_eth1, str(eth1delay) + "ms"))
        self.menu.append(("ETH1LOSS>>" , tMenu, loss_eth1,  str(eth1loss) + "%" ))
        self.refresh_menu()

class menu_addr(menu):
    # IPアドレス設定用
    def postinit(self):
        self.menu.append(("IPV4ADDR>>", tMenu, set_addr, ipaddr))
        self.menu.append(("NETMASK>>" , tMenu, set_netmask, netmask))
        self.menu.append(("GATEWAY>>", tMenu, set_gw, defaultgw))
        self.menu.append(("commit", tFunc, self.commit, ""))
        self.refresh_menu()

    # IPアドレスに関しては、設定後にCommitしたいので、こういう関数を作っている
    def commit(self):
        run_setnetworks()

class menu_reboot(menu):
    def postinit(self):
        self.menu.append(("REBOOT>", tFunc, self.cancel, "CANCEL"))
        self.menu.append(("REBOOT", tFunc, self.commit, "COMMIT"))
        self.refresh_menu()

    def cancel(self):
        return(False)
    def commit(self):
        print "REBOOT"
        pass


def is_num(k):
    # 関数あった気がするけど、インターネットがなかったので、とりあえず。。。
    for i in range(10):
        if k == str(i):
            return(True)
    return(False)

class menu_num(menu):
    """
    数字を受け付ける系の子メニュー

    modeが0の間は入力を受け付ける。
    maxlenまで数字を受け付けるとmodeは1となり、それ以上の入力を受け付けない
    *押されたらACするとかでもいいかもなー。
    """
    pos = 0
    mode = 0
    addr = ""
    target = ""
    maxlen = 4
    suffix = ""

    def postinit(self):
        pass

    def inkey(self, k):
        # キー入力を受け取った時

        # mode = 0 なら入力を受け取る
        if self.mode == 0:
            if is_num(k):
                self.addr += k
                self.pos += 1
            # 文字数が最大の場合、modeを1にする
            if self.pos == self.maxlen:
                print self.addr
                self.mode = 1
        if k == "#":
            return(self.save())
        elif k == "*":
            return(False)
        # 変更をディスプレイに出力
        self.refresh_out()

    def save(self):
        # #が押された時の処理。
        # save_mainを呼んで終了する
        stMs = "".join(self.addr)
        if stMs == "":
            stMs = "0"
        ms = int(stMs)
        self.save_main(ms)
        return(False)

    def save_main(self, ms):
        pass

    def refresh_out(self):
        # 2行目に現在の状況を表示
        global line2
        addrstr = "".join(self.addr) + self.suffix
        line2 = addrstr
        line_out()

class delay_eth0(menu_num):
    # DELAY eth0
    def postinit(self):
        global line1
        self.target = "eth0"
        line1 = "SET:DELAYETH0"
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
        line1 = "SET:DELAYETH1"
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
        line1 = "SET:LOSSETH0"
        self.suffix = "%"
        self.maxlen = 3
        line_out()

    def save_main(self, rate):
        # ロスレートに関しては100以上の値だったら100にする
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
        line1 = "SET:LOSSETH1"
        self.suffix = "%"
        self.maxlen = 3
        line_out()

    def save_main(self, rate):
        # ロスレートに関しては100以上の値だったら100にする
        global eth1loss
        if rate > 100:
            rate = 100
        print "commit: eth1loss:" + str(rate)
        eth1loss = rate
        run_settc()
        return(True)

class set_addr(menu):
    """
    アドレス系の制御クラス
    """
    def postinit(self):
        global line1
        line1 = "SET:BR0ADDR"
        self.pos = 0
        self.addr = ["0"] * 12
        line_out()
        self.refresh_ipaddr()

    def inkey(self, k):
        if is_num(k):
            self.addr[self.pos] = k
            self.pos += 1
        if self.pos == 12:
            print self.addr
            self.save()
            return(False)
        self.refresh_ipaddr()

    def refresh_ipaddr(self):
        global line2
        addrstr = ".".join(map(lambda x: "".join(x), zip(*[iter(self.addr)]*3)))
        line2 = addrstr
        line_out()

    def save(self):
        addrstr = ".".join(map(lambda x: str(int("".join(x))), zip(*[iter(self.addr)]*3)))
        self.save_main(addrstr)

    def save_main(self, addrstr):
        global ipaddr
        print "try:" + addrstr
        try:
            addr = ipaddress.ip_address(addrstr)
        except:
            print("error!")
            return(False)
        else:
            print("ok: " + addrstr)
        ipaddr = addrstr

class set_netmask(set_addr):
    def postinit(self):
        global line1
        line1 = "SET:NETMASK"
        self.pos = 0
        self.addr = [""] * 12
        line_out()
        self.refresh_ipaddr()

    def save_main(self, addrstr):
        global netmask
        try:
            addr = ipaddress.ip_network("0.0.0.0/" + addrstr)
        except:
            print("error!")
            return(False)
        else:
            print("ok" + addrstr)
        netmask = addrstr

class set_gw(set_addr):
    def postinit(self):
        global line1
        line1 = "SET:GATEWAY"
        self.pos = 0
        self.addr = [""] * 12
        line_out()
        self.refresh_ipaddr()

    def save_main(self, addrstr):
        global defaultgw
        try:
            addr = ipaddress.ip_address(addrstr)
        except:
            print("error!")
            return(False)
        else:
            print("ok" + addrstr)
        defaultgw = addrstr

def line_out_acm():
    print line1
    print line2
    lcd.text(0, line1)
    lcd.text(1, line2)

# LCDは存在しなかったらエラーハンドリングするので標準で出力は液晶にする
lcd = acm1602()
line_out = line_out_acm

last_pad = ""
class key_pad():
    def __init__(self):
        self.lastkey = " "
        key.init_pin()
    def key_scan(self):
        s = key.key_scan()
        if self.lastkey == s:
            return("")
        else:
            self.lastkey = s
            return(s)

if __name__ == "__main__":
    print netset.addr
    print "a"
    
    parser = optparse.OptionParser()
    parser.add_option("-p", "--USEKEYPAD", dest="f_keypad", action="store_true", default=False)
    (options, args) = parser.parse_args()
    if options.f_keypad:
        kp = key_pad()
        key_scan = kp.key_scan

    isLoop = True
    m = menu()

    while isLoop:
        c = key_scan()
        time.sleep(0.05)
        if c == "":
            continue
        if c == ".":
            isLoop = False
        m.inkey(c)
