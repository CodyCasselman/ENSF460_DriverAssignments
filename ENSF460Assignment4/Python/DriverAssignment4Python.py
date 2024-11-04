import serial
import time
import pandas as pd
from plotly.subplots import make_subplots
import plotly.graph_objects as go

#open up a serial port. Set the baud rate to 9600 and for it to read from port 3
serial_port = serial.Serial(port= "COM3", baudrate = 9600, bytesize = 8, timeout =2, stopbits = serial.STOPBITS_ONE)
#FLAGS AND INSTANTIATIONS
begin_detecting = False     #flag to know whether to begin recording ADC values
start_time = 0
read_time = 10             #amount of seconds to read the ADC for
detection_times = []        #timestamps of when numbers were recorded
numbers_detected = ''       #string which holds all the values written from ADC
end_loop = False
#the resolution is defined as Vref/2^N where N is the # of bits
resolution = 3.0 / (2**10)
print("Press PB2 When Ready to Record")
while not end_loop:
    #Continuously read things coming in from UART. The program will only "activate" when is see BEGIN
    single_line = ''
    single_line = serial_port.readline()
    #Begin recording UART inputs if we see BEGIN
    if(single_line == b"BEGIN\n"):
        begin_detecting = True
        start_time = time.time()
        print("Now Detecting")
    #Skip over everything below unless we are detecting ADC input
    while(begin_detecting == True):
        #Stop detecting input at 10 seconds
        if(time.time() - start_time > read_time):
            end_loop = True
            break
        #reads uint16_t nums as single bytes till \n n stores in string
        number = serial_port.readline() 
        if ((number != b' \n') and (number != b'\n')) : #removes any '\n' without num captures
            #Converts string of received uint16_t num to ASCII and combines Rx nums into 1 string
            numbers_detected = numbers_detected + number.decode('Ascii')  
            #Append the time the value was retrieved to a list
            time_segment = time.time() - start_time
            detection_times.append(time_segment)
#The serial port is no longer being used
serial_port.close()
#Data Cleaning
numbers_detected = numbers_detected.replace('\x00','')  #\x00 seems to be sent with Disp2String()
numbers_detected = numbers_detected.strip() # remove unwanted chars and spaces 
numbers_detected = numbers_detected.split(' \n ')  # split string by \n n store in list
#Casting our list of strings into their values
voltages = [float(adcbuf) * resolution for adcbuf in numbers_detected] #Voltage = Buffer Value * Resolution
ADCBufferValue = [float(adcbuf) for adcbuf in numbers_detected]
#Creating a DataFrame
d = {'Voltage': voltages, 'ADC Buffer Value':ADCBufferValue, 'Time': detection_times}

dataFrame = pd.DataFrame(data= d)
#Store the data in a csv file
dataFrame.to_csv("ADCValues.csv", index=False)
#Creating plots
fig = make_subplots(
    rows=2, cols=1,
    subplot_titles=("Voltage vs Time", "ADC Buffer vs Time")
    )
#Line plot of Voltage vs Time
fig.add_trace(
    go.Scatter(x=dataFrame.get('Time'), y=dataFrame.get('Voltage')),
    row=1, col=1
)
fig.update_yaxes(title_text = "Voltage (V)", row=1, col=1)
fig.update_xaxes(title_text = "Time (s)", row=1, col=1)
#Line plot of ADC Buffer Values vs Time
fig.add_trace(
    go.Scatter(x=dataFrame.get('Time'), y=dataFrame.get('ADC Buffer Value')),
    row=2, col=1
)
fig.update_yaxes(title_text = "ADC Buffer Value", row = 2, col = 1)
fig.update_xaxes(title_text="Time (s)", row = 2, col = 1)

fig.update_layout(
    title_text = "Voltages Scanned over 10 Second Interval",
    showlegend = False,
    template = "plotly_dark"
)

print(dataFrame)


fig.show()



