/*
 * File:   Assignment4Main.c
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

#define PULSETIMER T1CONbits.TON
#define BLINKTIMER T2CONbits.TON

#include "xc.h"
#include "TimeDelay.h"
#include "IOs.h"
#include "clkChange.h"
#include "UART2.h"
#include "ADC.h"
#include <stdio.h>

typedef enum{
    OFF,
    ON,
    RECORD,
    TRANSMIT,
    BLINK
}state_t;

typedef enum{
    BUFFER,
    BLINKER
}timer_state_t;


state_t machineState;
state_t returnState;
timer_state_t timer2State;

uint8_t pb1Click;
uint8_t pb2Click;
uint8_t pb3Click;
uint16_t pb1History;
uint16_t pb2History;
uint16_t pb3History;


int main(void) {
    newClk(8); //set the clock speed
    
    // INITIALIZATION OF UART, IO, ADC and TIMERS   
    InitUART2();
    IOInit();
    TimerInit();    
    ADCInit();
    
    //INITIALIZATION OF VARIABLES
    pb1History = 0;
    pb2History = 0;
    pb3History = 0;
    
    pb1Click = 0;
    pb2Click = 0;
    pb3Click = 0;
    
    machineState = OFF;
    returnState = OFF;
    while(1) {
        //Universal Logic
        switch(machineState){
            case OFF:
                //Exit logic:
                if(pb1Click)
                {
                    pb1Click = 0;
                    machineState = ON;
                }
                if(pb2Click)
                {
                    pb2Click = 0;
                    machineState = BLINK;
                }
                if(pb3Click){
                    pb3Click = 0;
                    machineState = RECORD;
                }
                //Entry Logic
                returnState = ON;
                
                //Main logic:
                
                
                
                break;
            case ON:
                //Exit Logic
                if(pb1Click)
                {
                    pb1Click = 0;
                    machineState = OFF;
                }
                if(pb2Click)
                {
                    pb2Click = 0;
                    machineState = BLINK;
                }
                if(pb3Click)
                {
                    pb3Click = 0;
                    machineState = RECORD;
                }
                
                //Entry Logic
                returnState = ON;
                
                //Main Logic
                
                
                break;
            case BLINK:
                //Flip blink timer status
                BLINKTIMER ^= 1;
                //Return to where we came
                machineState = returnState;
                break;
            case RECORD:
                //Flip Transmit Timer
                
                //Return to where we came
                machineState = returnState;
                break;
            case TRANSMIT:
                
                //Return to where we came
                machineState = returnState;
                break;
        }
        Idle(); //Processor spends most of its time in Idle
    } 
    return 0;
}

void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void){
    //Timer 1 is responsible for putting the microcontroller back to idle after waiting 10 seconds
    IFS0bits.T1IF = 0; //Clear flag
    
}

void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void){
    //Timer 2 is responsible for waking the microcontroller to read and output adc input
    IFS0bits.T2IF = 0; //Clear flag
}

void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void){
    //Timer 3 is our buffer timer
    IFS0bits.T3IF = 0; //Clear flag
    TIMER3 = 0; //Turn timer off
    //isolate the most recent button states. Change the flag about whether the button was clicked
    if((pb1History & 0b11) == 0b10)
    {
        pb1Click = 1;
        pb1History = 0;
    }
    if((pb2History & 0b11) == 0b10)
    {
        pb2Click = 1;
        pb2History = 0;
    }
    if((pb3History & 0b11) == 0b10)
    {
        pb3Click = 1;
        pb3History = 0;
    }
}

void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void){
    //CN interrupt should start the buffer and collect inputs
    IFS1bits.CNIF = 0; //Clear flag 
    pb1History = (pb1History << 1) | (BUTTON_1 ^ 1);
    pb2History = (pb2History << 1) | (BUTTON_2 ^ 1);
    pb3History = (pb3History << 1) | (BUTTON_3 ^ 1);
}
