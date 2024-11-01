# -*- coding: utf-8 -*-
"""
PYTHON DEMO -- PART 1
CAPTURE NUMERIC BYTES (CHAR) FROM MCU OVER A TIME INTERVAL
STORE CAPTURED BYTES IN A LIST AND CSV FILE
Created on Wed Nov  2 11:54:30 2022

@author: Rushi V
"""
import numpy as np
import math 
import csv
import serial  # pip install pyserial  or conda install pyserial
import time

## OPEN SERIAL PORT N CAPTURE FIXED NUMBER OF BYTES - REPLACE COM4 WITH THE COM PORT ASSIGNED TO USB_UART MODULE ON YOUR COMPUTER
ser = serial.Serial(port= "COM4", baudrate = 9600, bytesize = 8, timeout =2, stopbits = serial.STOPBITS_ONE)


## IF CAPTURING FIXED NUMBER OF SERIAL BYTES USING UART
s=ser.read(100)  # reads incoming 100 serial bytes n store as string in s
ser.close()
print(s)  
sChar = s.decode('Ascii')  #convert bytes to ascii
print(sChar)


## DATA CLEAN UP
sChar = sChar.strip()  # removes any spaces or \n or \r from beginning and end of string
print(sChar)
sChar = sChar.split(' ')  #split up string by separator ' ' and stores in list
print(sChar)


## IF RECEIVED SERIAL BYTES ARE NUMBERS ONLY
sNum = [int(elem) for elem in sChar]  #convert list of char to list of int numbers
print(sNum)
sNum = np.array(sNum)  #converts list of numbers to np.array
print(sNum)


## SAVE NUMPY ARRAY TO CSV FILE IN SAME FOLDER AS THIS CODE
np.savetxt('UARTDATA6.csv', sNum, delimiter=',')
