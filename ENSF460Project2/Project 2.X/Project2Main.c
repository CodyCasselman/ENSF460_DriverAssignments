/*
 * File:    Project2Main.c
 * Authors: Cody C, Evan M, Keeryn J
 *
 */

// FBS
#pragma config BWRP = OFF               // Table Write Protect Boot (Boot segment may be written)
#pragma config BSS = OFF                // Boot segment Protect (No boot program Flash segment)

// FGS
#pragma config GWRP = OFF               // General Segment Code Flash Write Protection bit (General segment may be written)
#pragma config GCP = OFF                // General Segment Code Flash Code Protection bit (No protection)

// FOSCSEL
#pragma config FNOSC = FRC              // Oscillator Select (Fast RC oscillator (FRC))
#pragma config IESO = OFF               // Internal External Switch Over bit (Internal External Switchover mode disabled (Two-Speed Start-up disabled))

// FOSC
#pragma config POSCMOD = NONE           // Primary Oscillator Configuration bits (Primary oscillator disabled)
#pragma config OSCIOFNC = ON            // CLKO Enable Configuration bit (CLKO output disabled; pin functions as port I/O)
#pragma config POSCFREQ = HS            // Primary Oscillator Frequency Range Configuration bits (Primary oscillator/external clock input frequency greater than 8 MHz)
#pragma config SOSCSEL = SOSCHP         // SOSC Power Selection Configuration bits (Secondary oscillator configured for high-power operation)
#pragma config FCKSM = CSECMD           // Clock Switching and Monitor Selection (Clock switching is enabled, Fail-Safe Clock Monitor is disabled)

// FWDT
#pragma config WDTPS = PS32768          // Watchdog Timer Postscale Select bits (1:32,768)
#pragma config FWPSA = PR128            // WDT Prescaler (WDT prescaler ratio of 1:128)
#pragma config WINDIS = OFF             // Windowed Watchdog Timer Disable bit (Standard WDT selected; windowed WDT disabled)
#pragma config FWDTEN = OFF             // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))

// FPOR
#pragma config BOREN = BOR3             // Brown-out Reset Enable bits (Brown-out Reset enabled in hardware; SBOREN bit disabled)
#pragma config PWRTEN = ON              // Power-up Timer Enable bit (PWRT enabled)
#pragma config I2C1SEL = PRI            // Alternate I2C1 Pin Mapping bit (Default location for SCL1/SDA1 pins)
#pragma config BORV = V18               // Brown-out Reset Voltage bits (Brown-out Reset set to lowest voltage (1.8V))
#pragma config MCLRE = ON               // MCLR Pin Enable bit (MCLR pin enabled; RA5 input pin disabled)

// FICD
#pragma config ICS = PGx2               // ICD Pin Placement Select bits (PGC2/PGD2 are used for programming and debugging the device)

// FDS
#pragma config DSWDTPS = DSWDTPSF       // Deep Sleep Watchdog Timer Postscale Select bits (1:2,147,483,648 (25.7 Days))
#pragma config DSWDTOSC = LPRC          // DSWDT Reference Clock Select bit (DSWDT uses LPRC as reference clock)
#pragma config RTCOSC = SOSC            // RTCC Reference Clock Select bit (RTCC uses SOSC as reference clock)
#pragma config DSBOREN = ON             // Deep Sleep Zero-Power BOR Enable bit (Deep Sleep BOR enabled in Deep Sleep)
#pragma config DSWDTEN = ON             // Deep Sleep Watchdog Timer Enable bit (DSWDT enabled)

// #pragma config statements should precede project file includes.
//Defines to improve readability
#define BUTTON_1 PORTAbits.RA2
#define BUTTON_2 PORTBbits.RB4
#define BUTTON_3 PORTAbits.RA4
#define LED LATBbits.LATB8
#define TIMER1 T1CONbits.TON
#define TIMER2 T2CONbits.TON
#define TIMER3 T3CONbits.TON

#include "xc.h"
#include "TimeDelay.h"
#include "IOs.h"
#include "clkChange.h"
#include "UART2.h"
#include "ADC.h"
#include <stdio.h>

//Define states
typedef enum { OFF, ON } State;
State mode = OFF;  //Initial state is off

uint8_t blinking;   //Whether the led should be blinking
uint8_t enterStateFlag, changeMode; //A flag to enter a state once, and a flag for when to change modes
uint8_t beginRecordingFlag, recordingFlag, endRecordingFlag;  //A flag for when recording begins, ends and if it is currently recording

//Counters for how long a blink has been going for...
//how long the system has been recording...
//and how many seconds the recording has been going for
uint32_t blink_time, recording_time, seconds_spent_recording;

uint32_t adcValue; //Reading from analog input

uint16_t PB1History, PB2History, PB3History; //Previous states of buttons

uint16_t cycleTime; //Time for total on/off cycle to dim the LED
uint32_t intensity; //Brightness wanted for PWM
uint32_t highTime; //Percentage of time on in PWM
uint32_t lowTime; //Percentage of time off in PWM
uint8_t led_high; //State of LED for PWM flipping

uint8_t adc_flag = 0; //Flag for when to take an ADC input. Slows down polling time

int main(void) {
    newClk(8); //set the clock speed
    
    // INITIALIZATION OF UART, IO, ADC and TIMERS   
    InitUART2();
    IOInit();
    TimerInit();    
    ADCInit();
    
    //Buttons have not been interacted with
    PB1History = 0;
    PB2History = 0;
    PB3History = 0;
    
    
    
    blinking = 0; 
    enterStateFlag = 1; //Enter a state upon program start
    changeMode = 0;     //Do not exit a state on program start
    recordingFlag = 0;
    recording_time = 0;
    seconds_spent_recording = 0;
    beginRecordingFlag = 0;
    endRecordingFlag = 1;
    cycleTime = 10000; //Pulse period is overall time section (In micro seconds) (0.01s)
    intensity = 50; 
    led_high = 0; //Since the LED starts off
    blink_time = 0;
    LED = 0;
    TIMER2 = 0;
    TIMER1 = 0;
    set_timerX_us(10000, 1);
    set_timerX_us(2*cycleTime, 2);
    set_timerX_us(100000, 3);
    while (1) {   
        switch (mode) {
            case ON:
                //While on, turn on timers for setting LED brightness and for polling the ADC
                TIMER1 = 1; 
                TIMER2 = 1;
                
                //If the ADC should be read (Timer 1 interrupted the program)
                if (adc_flag) {
                    adc_flag = 0;
                    adcValue = do_ADC();                        //Read the ADC
                    intensity = (adcValue * 100) / 1023;        //Find the intensity corresponding to it
                    if (blink_time < 500000 && blinking == 1)   //If currently on the falling edge of a blink...
                        intensity = 0;
                    if (intensity >= 100)                       //If the intensity is at its maximum, cap it just under
                        intensity = 99;
                    if (intensity <= 1)                         //If the intensity is 1 or under, clamp it to 0
                        intensity = 0;
                    highTime = (intensity * cycleTime) / 100;   //Calculate high and low times for PWM
                    lowTime = cycleTime - highTime;
                    
                    if (recordingFlag) {            //If currently recording...
                        if (!beginRecordingFlag)    
                        {
                            endRecordingFlag = 0;
                            beginRecordingFlag = 1;
                            Disp2String("BEGIN\n"); //Send begin if it just start to trigger the python script
                        }
                        Disp2Dec(intensity);        //Send the intensity and ADC value separated by a tab
                        Disp2String("\t"); 
                        Disp2Dec(adcValue);
                        Disp2String("\n");          //Send a new line to end the sequence and for python to know the end of the line
                    } else if (!endRecordingFlag)
                    {
                        endRecordingFlag = 1;
                        Disp2String("END\n"); //Send begin if it just start to trigger the python script
                    }
                    
                }
                
                if (changeMode && endRecordingFlag)     //If we should leave this mode upon PB1 being pressed change to mode = OFF. If the recording hasn't ended yet, don't leave
                {
                    changeMode = 0;
                    mode = OFF;
                    continue;
                }
                break;
                
            case OFF:
                if (blinking)   //If off, but blinking is turned on, the timers should be on
                {
                    TIMER2 = 1;
                    PR2 = (uint16_t)(500000 / 16);  //Set a constant rate of 0.5s for Timer 2 (Not variable like when in ON)
                } else {        //If off, and blinking is off, both timers should also be off
                    TIMER1 = 0;
                    TIMER2 = 0;
                    TMR2 = 0;
                    LED = 0;
                }
                
                if (changeMode)
                {
                    changeMode = 0;
                    mode = ON;
                    continue;
                }
                break;
        }
        Idle();
    }
    return 0;
}

//Timer 1 controls the length of a blink, the time spent recording and triggers the reading of the ADC at a set interval
void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void){
    IFS0bits.T1IF = 0; //Clear flag
    
    //Calculate time spent recording
    if (recordingFlag)
    {
        recording_time += 10000;
        if (recording_time > 1000000) {
            seconds_spent_recording += 1;
            recording_time = 0;
        }
        
        //Once 60 seconds have passed turn off recording
        if (seconds_spent_recording > 59){
            recordingFlag = 0;
            beginRecordingFlag = 0;
            recording_time = 0;
            seconds_spent_recording = 0;
        }
    } else {
        beginRecordingFlag = 0;
        recording_time = 0;
        seconds_spent_recording = 0;
    }
    
    //Calculate time spent blinking
    if (blinking)
        blink_time += 10000;
    
    //Trigger reading ADC
    adc_flag = 1;
}

//Timer 2 controls the brightness of the LED
void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void){ 
    //Timer 2 is our flash timer
    IFS0bits.T2IF = 0;  //Clear flag
    led_high ^= 1;      //Every interrupt should change the state of the LED from high to low or vise versa
    
    //If the current mode is OFF and PB2 triggered blinking, the LED should 100% on or 100% off
    if (mode == OFF && blinking) 
        LED ^= 1;
    //If mode is OFF && blinking == 0, LED should be 0
    
    
    if (mode == ON) {
    //If the blinking is turned off, the logic for the brightness of the light should be used
    //or if time has reached above 500000 micro seconds, meaning the high end of the blink is happening, that should use the brightness logic too
        if (blink_time > 500000 || blinking == 0) 
        {
            if (intensity == 0) LED = 0;    //LED is off if at minimum brightness
            else if (intensity == 99) LED = 1;    //LED is on if at maximum brightness

            //If brightness is between 1 and 99 then high and low time is used
            else if (!led_high) {       
                LED = 1;
                PR2 = (uint16_t)highTime / 16;  //Set the timer length for the next part of the brightness cycle
            } else {
                LED = 0;
                PR2 = (uint16_t)lowTime / 16;
            }

            if (blink_time > 1000000){ //If the full 1 second cycle of the blink has passed reset it to zero
                blink_time = 0;
                LED = 0;
            }
        } else {
            LED = 0;
        }
    }
}

void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void){
    //Timer 3 is our buffer timer
    IFS0bits.T3IF = 0; //Clear flag
    TIMER3 = 0; //Turn timer off
    //isolate the most recent button states. See if it was on and then off
    if((PB1History & 0b11) == 0b10)
    {
        changeMode = 1; //CHANGE STATE
        if (recordingFlag)  //If currently recording, stop
            recordingFlag = 0;
        PB1History = 0; //Reset PB1History
    }
    if((PB2History & 0b11) == 0b10)
    {
        blinking ^= 1; //Activate or deactivate blinking
        PB2History = 0; //Reset PB2History
    }
    if((PB3History & 0b11) == 0b10)
    {
        if (mode == ON) {
            recordingFlag ^= 1; //Start or stop reading on PB3 press        
        }
        PB3History = 0; //Reset PB2History
    }
}

void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void){
    //CN interrupt should start the buffer and collect inputs
    IFS1bits.CNIF = 0; //Clear flag 
    //Left shift button history, concatenate with current button value
    PB1History = (PB1History << 1) | (BUTTON_1 ^ 1);
    PB2History = (PB2History << 1) | (BUTTON_2 ^ 1);
    PB3History = (PB3History << 1) | (BUTTON_3 ^ 1);
    //Start buffer timer and reset timer value
    TMR3 = 0;
    TIMER3 = 1;
}
