#!/usr/bin/env python
# -*- coding: utf-8 -*-

from flask import Flask, request, url_for
import os

header = """
<html><head><title>RasberryPi i2c control</title></head>
<body>
"""

footer = """
</body></html>
"""

cmd_i2cset = "/usr/sbin/i2cset"

app = Flask(__name__)

@app.route('/', methods=['GET'])
def index():
    c = header
    c+= "<h1>RasPI i2c Server</h1>"
    c+= footer 
    return(c, 200)

@app.route('/hello', methods=['GET'])
def hello():
    print(request.args)
    name = request.args.get("name", "")
    print("<br>\n")
    c = ""
    c += "<hr>"
    c += "hello" + name
    c += "<hr>"
    return(c, 200)

@app.route('/set', methods=['GET'])
def hello_withname():
    bus = request.args.get("bus", "")
    value = request.args.get("value", "")
    if bus == "" or value == "":
        return("<font color=red>need bus and value</font>", 200)
    cmd = "{0} -y 1 0x{1} {2}".format(cmd_i2cset, bus, value)
    res = os.system(cmd)
    #res = "hoge"
    c = header
    c+= "<h1>exec mode</h1>"
    c+= "exec: {0} <br>".format(cmd)
    c+= "result: {0} <br>".format(res)
    c+= footer 
    return(c, 200)

with app.test_request_context():
    print url_for('hello')
    #print url_for('set')

if __name__ == '__main__':
    app.run(debug=True, host="0.0.0.0")
