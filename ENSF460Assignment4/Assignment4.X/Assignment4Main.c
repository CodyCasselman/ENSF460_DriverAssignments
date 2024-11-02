/*
 * File:   Assignment4Main.c
 * Author: Cody C, Evan M, Keeryn J
 *
 * Created on October 28, 2024, 1:18 PM
 */

//BELOW CODE COPIED FROM PROJECT 1 MAIN
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

uint16_t adc_value;
uint16_t mode;
uint16_t button_event;
uint16_t enter_state_flag;
uint16_t mode0_rate;
uint16_t mode1_rate;
uint16_t PBX_event;
uint16_t startBuffer;
uint16_t pb1_down;
uint16_t pb1_history;
uint16_t pb2_down;
uint16_t pb2_history;
uint16_t start_sending_flag;

int main(void) {
    AD1PCFG = 0xFFEF; //Read from AN5
    newClk(500);
    IPC4bits.CNIP = 6;
    IFS1bits.CNIF = 0;
    IEC1bits.CNIE = 1;
        
    
    // INITIALIZATION OF UART, IO, ADC and TIMERS   
    InitUART2();
    IOInit();
    TimerInit();    
    ADCInit();
    
    //Set rate to check ADC input
    mode0_rate = 1000;
    mode1_rate = 500; 
    
    //INITIAL VALUES
    delay_ms(10, 3); //TIMER3 WILL ALWAYS BE 10ms
    
    delay_ms(mode0_rate, 2);
    mode = 0;
    enter_state_flag = 1;
    
    startBuffer = 0;
    PBX_event = 0;
    //FOR MODE SWITCHING
    pb1_down = 0;
    pb1_history = 0;
    //PYTHON VALUES
    pb2_down = 0;
    pb2_history = 0;
    start_sending_flag = 0;
    
    while(1) {
        if (startBuffer)        //If the CN interrupt has turned on the input buffer
        {
            TMR3 = 0;           //Reset the timer
            TIMER3 = 1;         //Turn it on 
        }
        if(PBX_event)
        {
            //capturing inputs after a PBX event
            pb1_history = pb1_down;
            pb1_down = BUTTON_1 ^ 1;
            pb2_history = pb2_down;
            pb2_down = BUTTON_2 ^ 1;
            //reset pbx_event
            PBX_event = 0;
            startBuffer = 0;
        }
        if (!startBuffer) {
            switch(mode) {
                case 0: //Realterm
                    //STATE SWITCHING LOGIC
                    if(!pb1_down && pb1_history)
                    {
                        //change the state and turn off the timer
                        mode = 1;
                        enter_state_flag = 1;
                        TIMER2 = 0;
                        break;
                    }
                    //Initial Mode 0 output
                    if(enter_state_flag){
                        Disp2String("MODE 0: \r");
                        
                        //Turn on timer for reading potentiometer
                        delay_ms(mode0_rate, 2);
                        TMR2 = 0;
                        TIMER2 = 1;
                        enter_state_flag = 0;
                    }
                    //Find star amount and spaces after
                    adc_value = do_ADC();
                    uint16_t max_stars = 64;
                    uint16_t num_stars = (adc_value + 1) / 16;
                    if (num_stars < 1) { //Lowest reading
                        num_stars = 1;
                    }
                    uint16_t spaces = max_stars - num_stars;

                    //Print stars to Realterm, account for previous star line clearing
                    Disp2String("\033[8C");
                    for (uint16_t i = 0; i < num_stars; i++) {
                        Disp2String("*");
                    }

                    Disp2Hex(adc_value); //Potentiometer reading output

                    for (uint16_t j = 0; j < spaces; j++) {
                        Disp2String(" ");
                    }

                    Disp2String("\r");
                    break;
                case 1: //Python
                    //STATE EXIT LOGIC
                    if(!pb1_down && pb1_history)
                    {
                        mode = 0;
                        enter_state_flag = 1;
                        break;
                    }
                    if(enter_state_flag)
                    {
                        Disp2String("MODE 1:");
                        delay_ms(mode1_rate, 2);
                        TMR2 = 0;
                    }
                    if(pb2_down == 0 && pb2_history == 1)
                    {
                        start_sending_flag == 1;
                    }
                    break;
                default:
                    break;
            };     
        }               
        Idle();
    } 
    return 0;
}

void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void){
    
    IFS0bits.T2IF = 0; //Clear flag
}

void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void){
    
    IFS0bits.T3IF = 0; //Clear flag
    PBX_event = 1;
}

void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void){
    IFS1bits.CNIF = 0; //Clear flag 
    startBuffer = 1;
}