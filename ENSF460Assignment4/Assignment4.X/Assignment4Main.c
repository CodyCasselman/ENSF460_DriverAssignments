/*
 * File:   Assignment4Main.c
 * Author: Cody C, Evan M, Keeryn J
 *
 * Created on October 28, 2024, 1:18 PM
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
#define TIMER2 T2CONbits.TON
#define TIMER3 T3CONbits.TON

#include "xc.h"
#include "TimeDelay.h"
#include "IOs.h"
#include "clkChange.h"
#include "UART2.h"
#include "ADC.h"
#include <stdio.h>

uint16_t mode0Rate;
uint16_t mode1Rate;

uint8_t startBuffer;
uint8_t mode;
uint8_t enterStateFlag;
uint8_t exitStateFlag;
uint8_t beginRecordingFlag;
uint8_t recordingFlag;

uint16_t adcValue;

uint16_t PB1History;
uint16_t PB2History;

int main(void) {
    newClk(8); //set the clock speed
    
    // INITIALIZATION OF UART, IO, ADC and TIMERS   
    InitUART2();
    IOInit();
    TimerInit();    
    ADCInit();
    delay_ms(50, 3);
    //Set rate to check ADC input
    mode0Rate = 10;
    mode1Rate = 10; 
    //PB1 and PB2 have not been interacted with
    PB1History = 0;
    PB2History = 0;
    
    startBuffer = 0;
    enterStateFlag = 1;
    exitStateFlag = 0;
    beginRecordingFlag = 0;
    recordingFlag = 0;
    
    while(1) {
        if (!startBuffer) {
            switch(mode) {
                case 0: //Realterm
                    //STATE EXIT LOGIC
                    if(exitStateFlag){
                        mode = 1;
                        enterStateFlag = 1;
                        exitStateFlag = 0;
                        TIMER2 = 0;
                        continue;
                    }
                    //STATE ENTRY LOGIC
                    if(enterStateFlag){
                        enterStateFlag = 0;
                        Disp2String("\33[2K\r");
                        Disp2String("MODE 0: \r");
                        
                        //Change timer2 config. Turn it on
                        delay_ms(mode0Rate, 2);
                        TMR2 = 0;
                        TIMER2 = 1;
                    }
                    
                    print_stars();
                    
                    break;
                case 1: //Python
                    //STATE EXIT LOGIC
                    if(exitStateFlag){
                        //change state
                        mode = 0;
                        //change flag values
                        enterStateFlag = 1;
                        exitStateFlag = 0;
                        beginRecordingFlag = 0;
                        recordingFlag = 0;
                        TIMER2 = 0;
                        continue;
                    }
                    //STATE ENTRY LOGIC
                    if(enterStateFlag){
                        //print something to the terminal
                        Disp2String("\33[2K\r");
                        Disp2String("MODE 1: CLOSE PORT AND TURN ON PYTHON\r");
                        enterStateFlag = 0;
                        //change timer delay
                        delay_ms(mode1Rate, 2);
                        TMR2 = 0;
                    }
                    if(beginRecordingFlag){
                        Disp2String("BEGIN\n");
                        beginRecordingFlag = 0;
                        recordingFlag = 1;
                        TIMER2 = 1;
                    }
                    if(recordingFlag){
                        adcValue = do_ADC();
                        Disp2Dec(adcValue);
                        Disp2String("\n");
                    }
                    break;
            };  
        }               
        Idle();
    } 
    return 0;
}

void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void){
    //Timer 2 is responsible for waking the microcontroller to read and output adc input
    IFS0bits.T2IF = 0; //Clear flag
}

void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void){
    //Timer 3 is our buffer timer
    IFS0bits.T3IF = 0; //Clear flag
    TIMER3 = 0; //Turn timer off
    startBuffer = 0; //Reset buffer flag
    //isolate the most recent button states. See if it was on and then off
    if((PB1History & 0b11) == 0b10)
    {
        exitStateFlag = 1; //CHANGE STATE
        PB1History = 0; //Reset PB1History
    }
    if((PB2History & 0b11) == 0b10)
    {
        beginRecordingFlag = 1; //Begin recording data for python
        PB2History = 0; //Reset PB2History
    }
}

void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void){
    //CN interrupt should start the buffer and collect inputs
    IFS1bits.CNIF = 0; //Clear flag 
    //Left shift button history, concatenate with current button value
    PB1History = (PB1History << 1) | (BUTTON_1 ^ 1);
    PB2History = (PB2History << 1) | (BUTTON_2 ^ 1);
    //Start buffer timer and reset timer value
    startBuffer = 1;
    TMR3 = 0;
    TIMER3 = 1;
}

void print_stars(){
    adcValue = do_ADC();
                    
    //Find star amount and spaces after
    uint16_t max_stars = 64;
    uint16_t num_stars = (adcValue + 1) / 16;
    if (num_stars < 1) { //Lowest reading
        num_stars = 1;
    }
    uint16_t spaces = max_stars - num_stars;

    //Print stars to Realterm, account for previous star line clearing
    Disp2String("\033[8C");
    for (uint16_t i = 0; i < num_stars; i++) {
        Disp2String("*");
    }
    Disp2Hex(adcValue); //Potentiometer reading output
    for (uint16_t j = 0; j < spaces; j++) {
        Disp2String(" ");
    }
    Disp2String("\r");
}