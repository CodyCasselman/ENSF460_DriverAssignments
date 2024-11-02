/*
 * File:   TimeDelay.c
 * Author: Cody Casselman, Keeryn Johnson, Evan Mann
 *
 * Created on September 16, 2024, 4:05 PM
 */

#include "TimeDelay.h"
#include "clkChange.h"
#include "xc.h"

#define BUTTON_1 PORTAbits.RA2
#define BUTTON_2 PORTBbits.RB4
#define BUTTON_3 PORTAbits.RA4
#define LED LATBbits.LATB8
#define TIMER2 T2CONbits.TON
#define TIMER3 T3CONbits.TON

uint32_t timer2_prescale = 64;
uint32_t timer3_prescale = 1;
uint32_t timer_freq = 500000;
uint32_t ms_to_s_factor = 1000;

//INITIALIZE THE TIMERS
void TimerInit() {
    T2CONbits.T32 = 0; //16-Bit timers
    //TIMER 3 CONFIG SETTINGS
    T3CONbits.TCKPS = 0;   //Prescaler for Timer 3 set to 1:1.
    T3CONbits.TCS = 0;     //Timer 3 clock source is the internal clock (Fcy).
    T3CONbits.TSIDL = 0;   //Timer 3 continues running in idle mode.
    IPC2bits.T3IP = 2;     //Interrupt priority for Timer 3 set to 2.
    IFS0bits.T3IF = 0;     //Clear Timer 3 interrupt flag.
    IEC0bits.T3IE = 1;     //Enable Timer 3 interrupt.
    //TIMER 2 CONFIG SETTINGS
    T2CONbits.TCKPS = 2;   //Prescaler for Timer 2 set to 1:64.
    T2CONbits.TCS = 0;     //Timer 2 clock source is the internal clock (Fcy).
    T2CONbits.TSIDL = 0;   //Timer 2 continues running in idle mode.
    IPC1bits.T2IP = 2;     //Interrupt priority for Timer 2 set to 2.
    IFS0bits.T2IF = 0;     //Clear Timer 2 interrupt flag.
    IEC0bits.T2IE = 1;     //Enable Timer 2 interrupt.
    //Reset both timers and turn them off
    TMR2 = 0;
    TIMER2 = 0;
    
    TMR3 = 0;
    TIMER3 = 0;
}

void delay_ms(uint32_t time_ms, uint8_t timerNum){ //0 <= time_ms <= 986880ms
    if(timerNum == 2){
        //Change timer2 PR value
        PR2 = (uint16_t)calculate_pr(time_ms, timer2_prescale);
    }
    else if(timerNum == 3){
        //Change timer3 PR value
        PR3 = (uint16_t)calculate_pr(time_ms, timer3_prescale);
    }
}

uint32_t calculate_pr(uint32_t time_ms, uint32_t prescale)
{   
    return ((uint64_t)time_ms * (uint64_t)timer_freq / (uint64_t)(ms_to_s_factor * 2 * prescale));
}


