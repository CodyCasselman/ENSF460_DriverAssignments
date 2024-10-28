/* 
 * File:   TimeDelay.h
 * Author: keeryn
 *
 * Created on October 3, 2024, 7:57 PM
 */

#ifndef TIMEDELAY_H
#define TIMEDELAY_H

#include <xc.h>

// Function declarations
void delay_ms(uint16_t time_ms);
uint32_t calculate_pr(uint16_t time_ms, uint16_t test_prescale);
uint16_t _find_prescale(uint16_t time_ms);

#endif