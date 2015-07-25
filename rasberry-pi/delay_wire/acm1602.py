#!/usr/bin/python
# -*- coding: utf-8 -*-

import subprocess

"""
i2csetは成功すると0
失敗すると1を返す
"""

class acm1602(object):
    """
    ACM1602用の制御コード
    """
    lastline1 = [" "] * 16
    lastline2 = [" "] * 16


    def dprint(self, str):
        """
        Debug Print
        """
        if self.debug:
            print(str)
    
    def __init__(self, isdebug = False):
        # LCDの初期化をトライ
        res = self._sendcmd("0x01")
        self.debug = isdebug
        if res == 0:
            self.dprint("LCD: init OK")
        else:
            # LCDの初期化に失敗 = 接続できていないとき
            # コマンドを何も送らないようにする
            self._run = self._runnothing
            pass
        
        self._sendcmd("0x38")
        self._sendcmd("0x0e")
        self._sendcmd("0x06")

    def _sendcmd(self, str):
        # 制御コード
        return self._run("sudo i2cset -y 1 0x50 0x00 {0} b".format((str)))

    def _sendtext(self, str):
        # データコード
        return self._run("sudo i2cset -y 1 0x50 0x80 {0} b".format((str)))
    
    def _run(self, str):
        # コマンドの実行
        # subprocess.callはリストでコマンドを送るのでsplitする
        res = subprocess.call(str.strip().split(" "))
        return(res)
    
    def _runnothing(self, str):
        # ACM1602が接続されていない時のダミーコード
        pass
    
    def text(self, line, text):
        # 16文字なかったらとりあえず16文字埋める
        exstr = lambda x: x + " " * (16 - len(x))
        text = exstr(text[0:16])
        # 逆に256文字くらいあったら、一回そこで切る
        s = text[0:256]
        stDisp = [" "] * 16
        count = 0
        # それで、表示できない文字は「ないもの」とする
        # 例: abあいうcd -> abcd
        for c in s:
            if 0x20 <= ord(c) and ord(c) <= 0x7e:
                stDisp[count] = c
                count += 1
            if count == 16:
                break
        # 1行目か2行目かによって、制御コードのオフセットを変える
        if line == 0:
            offset = 0x80
            laststr = self.lastline1
        elif line == 1:
            offset = 0xc0
            laststr = self.lastline2
        # 各文字について、最後のデータと異なっているときのみデータを送る
        # 尚、0x80 = 1行目の１文字目, 0xc0 = 2行目の1文字目なので、それにpositionを足す
        for p in range(16):
            if stDisp[p] != laststr[p]:
                self._sendcmd(hex(offset + p))
                self._sendtext(ord(stDisp[p]))
        # 終了後、バッファに値を記録
        if line == 0:
            self.lastline1 = stDisp
        elif line == 1:
            self.lastline2 = stDisp

            
    def cls(self):
        # 画面リセット
        self._sendcmd("0x01")
        self.lastline1 = [" "] * 16
        self.lastline2 = [" "] * 16

if __name__ == "__main__":
    lcd = acm1602(isdebug = True)
    lcd.text(0, "hogehogehogehogehogehogegegeaaaaaaaaaaa")
    lcd.text(1, "hogehogehogehogehogehogegegeaaaaaaaaaaa")
    lcd.text(0, "mogemoge1234hogehogehogegegeaaaaaaaaaaa")
    lcd.text(1, "acge")
