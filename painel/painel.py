#!/usr/bin/python

import urllib2
import serial
import json
import time

ser = serial.Serial('/dev/ttyACM0', 9600)

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
        time.sleep(5)

    lt = time.localtime()
    ser.write("  %02d:%02d:%02d" % (lt.tm_hour, lt.tm_min, lt.tm_sec))
    time.sleep(5)
    ser.write("%02d/%02d/%4d" % (lt.tm_mday, lt.tm_mon, lt.tm_year))
    time.sleep(5)

    url = 'http://api.openweathermap.org/data/2.5/weather?q=florianopolis'
    temp = json.loads(urllib2.urlopen(url).read())['main']['temp']-273.15
    ser.write("   %.4g C" % (temp))
    time.sleep(5)
