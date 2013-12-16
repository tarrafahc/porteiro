#!/usr/bin/python

import urllib2
import serial
import json
import time
import os

def botao():
    bytes = ser.read(1)
    if bytes:
        if   bytes is '1':
            print "Tarrafa is on"
            os.system("ssh -i botao-tarrafa tarrafahc@tarrafa.net 1")
        elif bytes is '0':
            print "Tarrafa is off"
            os.system("ssh -i botao-tarrafa tarrafahc@tarrafa.net 0")

ser = serial.Serial('/dev/ttyUSB0', baudrate=9600, timeout=1)

time.sleep(1)

while True:
    text = ""
    try:
        fp = open('text', 'r')
        text = "%10s" % (fp.read())
        fp.close()
    except IOError:
        pass

    if text != "":
        ser.write(text)
        botao()

    lt = time.localtime()
    ser.write("  %02d:%02d:%02d" % (lt.tm_hour, lt.tm_min, lt.tm_sec))
    botao()
    ser.write("%02d/%02d/%4d" % (lt.tm_mday, lt.tm_mon, lt.tm_year))
    botao()

#    url = 'http://api.openweathermap.org/data/2.5/weather?q=florianopolis'
#    temp = json.loads(urllib2.urlopen(url).read())['main']['temp']-273.15
#    ser.write("   %.4g C" % (temp))
#    time.sleep(5)
