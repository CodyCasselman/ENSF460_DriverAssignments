import serial
import time
import pandas as pd
from plotly.subplots import make_subplots
import plotly.graph_objects as go

def clean_data(messy_list):
    messy_list = messy_list.replace('\x00','')  # \x00 seems to be sent with Disp2String()
    messy_list = messy_list.strip()             # remove unwanted chars and spaces 
    messy_list = messy_list.split(' \n ')       # split string by \n n store in list
    messy_list = [float(buf) for buf in messy_list]
    return messy_list

def readUART():
    #FLAGS AND INSTANTIATIONS
    begin_detecting = False         #flag to know whether to begin recording ADC values
    start_time = 0
    read_time = 60                  #amount of seconds to read the ADC for
    detection_times = []            #timestamps of when numbers were recorded
    adcValues_detected = ''         #string which holds all the values written from ADC
    intensityValues_detected = ''   #string which holds all the intensity values
    end_loop = False
    serial_port = serial.Serial(port= "COM3", baudrate = 9600, bytesize = 8, timeout =2, stopbits = serial.STOPBITS_ONE)
    while not end_loop:
        #open up a serial port. Set the baud rate to 9600 and for it to read from port 3
        
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
            intensity_and_adcValue = serial_port.readline() 
            if (intensity_and_adcValue == b"\x00END\n"):    #If end is sent over UART when repressing PB3 or if it switches to off mode, stop recording
                end_loop = True
                print("Recording interupted")
                break

            if ((intensity_and_adcValue != b' \n') and (intensity_and_adcValue != b'\n')) : #removes any '\n' without num captures
                intensityValue = intensity_and_adcValue.split(b'\t')[0] #Read the intensity value on the left side of the tab its sent with
                adcValue = intensity_and_adcValue.split(b'\t')[1]       #Read the ADC value on the right side of the tab its sent with

                #values are stored in one string seperated by newlines
                adcValues_detected = adcValues_detected + adcValue.decode('Ascii')                          #Append the ADC value
                intensityValues_detected = intensityValues_detected + intensityValue.decode("Ascii") + "\n" #Addend the intensity value, with a \n since it doesn't have one

                #Append the time the value was retrieved to a list
                time_segment = time.time() - start_time
                detection_times.append(time_segment)
    #The serial port is no longer being used
    serial_port.close()
    
    #data cleaning
    intensities = clean_data(intensityValues_detected) 
    adcValues = clean_data(adcValues_detected)

    return intensities, adcValues, detection_times
            
if __name__ == "__main__":
    print("Press PB3 When Ready to Record. Press it again, or PB1 to stop")
    
    #This will open a serial port, read our values, and return cleaned lists of data
    intensities, adcValues, detection_times = readUART()

    #Creating a DataFrame
    d = {'Intensities': intensities, 'ADC Values':adcValues, 'Time': detection_times}
    dataFrame = pd.DataFrame(data= d)
    #Store the data in a csv file
    dataFrame.to_csv("UARTValues.csv", index=False)
    #Creating plots
    fig = make_subplots(
        rows=2, cols=1,
        subplot_titles=("Intensity vs Time", "ADC Values vs Time")
        )
    #Line plot of Intensities vs Time
    fig.add_trace(
        go.Scatter(x=dataFrame.get('Time'), y=dataFrame.get('Intensities')),
        row=1, col=1
    )
    fig.update_yaxes(title_text = "Intensity", row=1, col=1)
    fig.update_xaxes(title_text = "Time (s)", row=1, col=1)

    #Line plot of ADC Values vs Time
    fig.add_trace(
        go.Scatter(x=dataFrame.get('Time'), y=dataFrame.get('ADC Values')),
        row=2, col=1
    )
    fig.update_yaxes(title_text = "ADC Value", row = 2, col = 1)
    fig.update_xaxes(title_text="Time (s)", row = 2, col = 1)

    fig.update_layout(
        title_text = "Intensities and ADC values as functions of time",
        showlegend = False,
        template = "plotly_dark"
    )

    print(dataFrame)


    fig.show()



