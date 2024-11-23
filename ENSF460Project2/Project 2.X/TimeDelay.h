/* 
 * File:   TimeDelay.h
 * Authors: Cody C, Evan M, Keeryn J
 */

#ifndef TIMEDELAY_H
#define TIMEDELAY_H

#include <xc.h>

// Function declarations
void TimerInit();
void set_timerX_us(uint32_t time_us, uint8_t timerNum);
uint32_t calculate_pr(uint32_t time_us, uint32_t test_prescale);

#endif