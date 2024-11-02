import numpy as np
import math 
import csv
import serial  # pip install pyserial  or conda install pyserial
import time
import pandas as pd
import plotly.express as px

#open up a serial port. Set the baud rate to 4800 and for it to read from port 3
serial_port = serial.Serial(port= "COM3", baudrate = 4800, bytesize = 8, timeout =2, stopbits = serial.STOPBITS_ONE)

begin_detecting = False #flag to know whether to begin recording ADC values
detection_sequence = "BEGIN" #Specific phrase which will trigger ADC collection
start_time = 0
read_time = 10 #amount of seconds to read the ADC for
detection_times = [] #timestamps of when numbers were recorded
numbers_detected = '' #string which holds all the hex values written from ADC

while True:
    if(begin_detecting == False):
        single_byte = ''
        single_byte = serial_port.read()
        if(single_byte == "\n"):
            if(serial_port.readline() == detection_sequence):
                begin_detecting = True
                start_time = time.time()
    if(begin_detecting == True):
        if(time.time() - start_time > 10):
            break
        line = serial_port.readline() # reads uint16_t nums as single bytes till \n n stores in string
        if ((line != b' \n') and (line != b'\n')) : #removes any '\n' without num captures
            numbers_detected = numbers_detected + line.decode('Ascii')  # Converts string of received uint16_t num to ASCII and combines Rx nums into 1 string
            time_segment = time.time() - start_time # Time stamp received number
            detection_times.append(time_segment)

serial_port.close()
    