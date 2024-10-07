/*
 * File:   TimeDelay.c
 * Author: Cody Casselman, Keeryn Johnson, Evan Mann
 *
 * Created on September 16, 2024, 4:05 PM
 */

#include "TimeDelay.h"
#include "clkChange.h"
#include "xc.h"


uint16_t prescale;
uint32_t timer_freq = 32000;
uint32_t ms_to_s_factor = 1000;
uint16_t max_pr2_value = 0xffff;

void delay_ms(uint16_t time_ms){ //0 <= time_ms <= 986880ms
    TMR2 = 0;
    newClk(32); //change the clock frequency to 32kHz
    prescale = _find_prescale(time_ms);
    switch(prescale){
        case 1: // 1:1 prescale
            T2CONbits.TCKPS = 0;
            break;
        case 8: // 1:8 prescale
            T2CONbits.TCKPS = 1;
            break;
        case 64: // 1:64 prescale
            T2CONbits.TCKPS = 2;
            break;
        case 256: // 1:256 prescale
            T2CONbits.TCKPS = 3;
            break;
        default: // default to 1:256 prescale
            prescale = 256;
            T2CONbits.TCKPS = 3;
    }
    PR2 = calculate_pr2(time_ms, prescale);
    T2CONbits.TON = 1; //Turn the timer on
    Idle();
    newClk(500); //reset the clock frequency back to 500kHz 
}

uint32_t calculate_pr2(uint16_t time_ms, uint16_t test_prescale)
{   
    return (time_ms * timer_freq / (ms_to_s_factor * 2 * test_prescale));
}

uint16_t prescale_vals[4] = {1,8,64,256};

uint16_t _find_prescale(uint16_t time_ms)
{
    uint16_t prescale_val;
    for(uint8_t i = 0; i < 4; i++) //loop through possible prescales until you find one that results in a pr2 value less than 65535
    {
        prescale_val = prescale_vals[i];
        if(calculate_pr2(time_ms, prescale_val) < max_pr2_value){
            return prescale_val;
        }
    }
    return 256; //default prescale value of 256
}

