/*
 * File:   IOs.c
 * Author: Cody Casselman, Keeryn Johnson, Evan Mann
 *
 * Created on September 16, 2024, 3:47 PM
 */

#include "IOs.h"
#include "TimeDelay.h"
#include "xc.h"
void IOInit(){
    TRISBbits.TRISB8 = 0; //Set to output for light
    CNPU2bits.CN30PUE = 1; //Pull up for button 1
    CNPU1bits.CN1PUE = 1; //Pull up for button 2
    CNPU1bits.CN0PUE = 1; //Pull up for button 3
    TRISAbits.TRISA2 = 1; //setting input for RA2, RB4, and RA4
    TRISBbits.TRISB4 = 1;
    TRISAbits.TRISA4 = 1;
}

void IOCheck(){
    uint16_t delay_time = 0;
        if (PORTAbits.RA2 + PORTBbits.RB4 + PORTAbits.RA4 == 2 || (PORTAbits.RA2 + PORTBbits.RB4 == 0 && PORTAbits.RA4 == 1)) 
            //if one or more buttons are pressed or PB1 and PB2 are pressed
        {
            if(PORTAbits.RA2 + PORTBbits.RB4 == 0){ //if PB1 and PB2 are pressed
                delay_time = 1;
            }
            else if (PORTAbits.RA2 == 0){ //0.5 sec delay
                delay_time = 500;
            } 
            else if (PORTBbits.RB4 == 0) { //1 sec delay
                    delay_time = 1000;
            }
            else if (PORTAbits.RA4 == 0) { // 4 sec delay
                    delay_time = 4000;  
            }
            delay_ms(delay_time); // change the PR2/clkfreq/prescale value and turn the timer on to cause delay
        }
        else //if there's no input, turn the timer and light off
        {
            LATBbits.LATB8 = 0;
            T2CONbits.TON = 0;
            //BUG: By using Idle() but not using a CN interrupt, we can't update the value of LTFlag (in main)
        }
}
    
