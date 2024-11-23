/*
 * File:   TimeDelay.c
 * Authors: Cody C, Evan M, Keeryn J
 *
 */

#include "TimeDelay.h"
#include "clkChange.h"
#include "xc.h"

#define TIMER1 T1CONbits.TON
#define TIMER2 T2CONbits.TON
#define TIMER3 T3CONbits.TON

uint32_t timer1_prescale = 256;
uint32_t timer2_prescale = 64;
uint32_t timer3_prescale = 1;
uint32_t timer_freq = 8000000;
uint32_t us_to_s_factor = 1000000;

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
    //TIMER 1 CONFIG
    T1CONbits.TCKPS = 3;   //Prescaler for Timer 1 set to 1:256.
    T1CONbits.TCS = 0;     //Timer 1 clock source is the internal clock (Fcy).
    T1CONbits.TSIDL = 0;   //Timer 1 continues running in idle mode.
    IPC0bits.T1IP = 2;     //Interrupt priority for Timer 1 set to 2.
    IFS0bits.T1IF = 0;     //Clear Timer 1 interrupt flag.
    IEC0bits.T1IE = 1;     //Enable Timer 1 interrupt.
    //Reset all timers and turn them off
    TMR1 = 0;
    TIMER1 = 0;
    
    TMR2 = 0;
    TIMER2 = 0;
    
    TMR3 = 0;
    TIMER3 = 0;
}

void set_timerX_us(uint32_t time_us, uint8_t timerNum){ //0 <= time_ms <= 986880ms
    if(timerNum == 1){
        //Change timer1 PR value
        PR1 = (uint16_t)calculate_pr(time_us, timer1_prescale);
    }
    if(timerNum == 2){
        //Change timer2 PR value
        PR2 = (uint16_t)time_us / 16;
    }
    else if(timerNum == 3){
        //Change timer3 PR value
        PR3 = (uint16_t)calculate_pr(time_us, timer3_prescale);
    }
}

uint32_t calculate_pr(uint32_t time_us, uint32_t prescale)
{   
    return ((uint64_t)time_us * (uint64_t)timer_freq / (uint64_t)(us_to_s_factor * 2 * prescale));
}


