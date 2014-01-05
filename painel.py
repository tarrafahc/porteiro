#!/usr/bin/python
# -*- coding: utf-8 -*-

import urllib2
import serial
import json
import time
import os

# Lê um byte da serial pelo Arduino. Se tiver algum dado, atualiza o servidor.
def botao():
    bytes = ser.read(1)
    if bytes:
        if   bytes is '1':
            print "Tarrafa is on"
            os.system("ssh -i botao-tarrafa tarrafahc@tarrafa.net 1")
        elif bytes is '0':
            print "Tarrafa is off"
            os.system("ssh -i botao-tarrafa tarrafahc@tarrafa.net 0")

# Abrir a porta serial do Arduino em 9600 baud com tempo de espera de 1
# segundo. O tempo de espera é usado na função de leitura para não precisar
# fazer o código dormir explicitamente.
ser = serial.Serial('/dev/ttyUSB0', baudrate=9600, timeout=1)

# Esperar 1 segundo caso o Arduino queira reiniciar.
time.sleep(1)

# Loop principal do painel.
while True:
    # Mostrar texto de arquivo
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

    # Mostrar hora
    lt = time.localtime()
    ser.write("  %02d:%02d:%02d" % (lt.tm_hour, lt.tm_min, lt.tm_sec))
    botao()
    # Mostrar data
    ser.write("%02d/%02d/%4d" % (lt.tm_mday, lt.tm_mon, lt.tm_year))
    botao()

    # Mostrar temperatura
    # TODO pegar uma vez por hora ou coisa parecida
#    url = 'http://api.openweathermap.org/data/2.5/weather?q=florianopolis'
#    temp = json.loads(urllib2.urlopen(url).read())['main']['temp']-273.15
#    ser.write("   %.4g C" % (temp))
#    time.sleep(5)
