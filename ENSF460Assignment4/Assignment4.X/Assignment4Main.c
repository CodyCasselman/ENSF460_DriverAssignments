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

uint16_t value;
uint16_t mode;
uint16_t ratio;
uint16_t t2_flag;

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
    
    delay_ms(1000, 2);
    TIMER2 = 1;
    t2_flag = 0;
    mode = 0;
    
    while(1) {
        if (t2_flag) {
            switch(mode) {
                case 0: //Realterm
                        Disp2String("\033[2K"); //Clears line?
                        Disp2String("MODE 0: ");
                        value = do_ADC();
                        ratio = (value + 1) / 16;
                        
                        for (int i = 0; i < ratio; i++) {
                            Disp2String("*");
                        }

                        Disp2String(" "); //Space after 

                        Disp2Hex(value);
                        Disp2String("\r");
                        break;
                case 1: //Python
                    Disp2String("MODE 1: ");
                    break;
            };     
            
            t2_flag = 0;
        }
                        
        Idle();
    } 
    return 0;
}

void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void){
    //Don't forget to clear the timer 2 interrupt flag!
    IFS0bits.T2IF = 0;
    t2_flag = 1;
}

void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void){
    //Don't forget to clear the timer 3 interrupt flag!
    IFS0bits.T3IF = 0;
}

void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void){
    IFS1bits.CNIF = 0; //Clear flag 
    
    //Button down changes the mode
    mode ^= 1;
}