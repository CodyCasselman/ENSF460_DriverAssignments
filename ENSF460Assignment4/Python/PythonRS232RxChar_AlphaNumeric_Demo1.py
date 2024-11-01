# -*- coding: utf-8 -*-
"""
PYTHON DEMO -- PART 1
CAPTURE ALPHA-NUMERIC BYTES (CHAR) FROM MCU OVER A TIME INTERVAL
STORE CAPTURED BYTES IN A LIST AND CSV FILE
Created on Wed Nov  2 11:54:30 2022

@author: Rushi V
"""
import numpy as np
import math 
import csv
import serial  # pip install pyserial  or conda install pyserial
import time

## OPEN SERIAL PORT COM4 -  USE COM PORT ASSIGNED BY YOUR COMPUTER TO USB_UART MODULE
ser = serial.Serial(port= "COM4", baudrate = 9600, bytesize = 8, timeout =2, stopbits = serial.STOPBITS_ONE)


## CAPTURING BYTES OVER A TIME INTERVAL
sChar = '' # empty string to hold incoming bytes  
startTime = time.time()  #record start time
timeInterval = 0.5  # in sec 
while(time.time() - startTime < timeInterval):
    s=ser.read() # reads a single byte
    sChar = sChar + s.decode('Ascii') #convert to ASCII and append to string

ser.close()
print(sChar)


## DATA CLEAN UP
sChar = sChar.strip()  # removes any spaces or \n or \r from beginning and end of string
print(sChar)
listChar = sChar.split(' ')  #split string by ' ' and converts to list of char
print(listChar)


## DATA FORMATING
listChar = [elem +'\n' for elem in listChar]  #add \n after each list elem to save in diff rows
print(listChar)
sChar2 = ' '.join(listChar)  #converts list back to string for csv write of string


## WRITE ALPHA NUMERIC DATA STRING IN CSV. NUMPY BASED CSV WRITE  
file = open(file = 'UART9.csv', mode= 'w')
file.write(sChar2)
file.close()



