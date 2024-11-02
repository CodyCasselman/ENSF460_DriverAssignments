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

uint32_t timer2_prescale = 8;
uint32_t timer3_prescale = 64;
uint32_t timer_freq = 8000000;
uint32_t ms_to_s_factor = 1000;

void delay_ms(uint32_t time_ms, uint8_t timerNum){ //0 <= time_ms <= 986880ms
    if(timerNum == 2){
        PR2 = (uint16_t)calculate_pr(time_ms, timer2_prescale);
        T2CONbits.TCKPS = 1;
    }
    else if(timerNum == 3){
        PR3 = (uint16_t)calculate_pr(time_ms, timer3_prescale);
        T3CONbits.TCKPS = 2;
    }
}

uint32_t calculate_pr(uint32_t time_ms, uint32_t prescale)
{   
    return ((uint64_t)time_ms * (uint64_t)timer_freq / (uint64_t)(ms_to_s_factor * 2 * prescale));
}


