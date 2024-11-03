/*
 * File:   ADC.c
 * Authors: Cody C, Evan M, Keeryn J
 *
 */


#include "xc.h"
#include "ADC.h"
void ADCInit() {
    AD1PCFG = 0xFFDF; //Read from AN5
    
    AD1CON2bits.VCFG = 0b000; //Select voltage range of ADC 
    AD1CON3bits.ADCS = 0b11111; //Select analog conversion rate 
    AD1CON1bits.SSRC = 0b111; //Sets to auto conversion (Sets based on internal counter)
    AD1CON3bits.SAMC = 0b00001; //Set the Auto-Sample Time bits (0b11111 = 31Tad)
    AD1CON1bits.FORM = 00; //Sets data output format (Integer)
    AD1CON2bits.SMPI = 0b0000; //Throws interrupt each time it scans
    
    AD1CHSbits.CH0NA = 0; //Channel 0 negative input is Vr-
    AD1CHSbits.CH0SA = 0b0101; //Channel 0 positive input is AN5
}

uint16_t do_ADC(void)
{
    uint16_t ADCvalue ; // 16 bit register used to hold ADC converted digital output ADC1BUF0
    
    
    AD1CON1bits.ADON = 1; //Turns ADC on
    AD1CON1bits.SAMP = 1; //Start Sampling, Conversion starts automatically after SSRC and SAMC settings
    //Poll DONE bit until the sampling is finished
    while(AD1CON1bits.DONE==0) 
        {}
    ADCvalue = ADC1BUF0; // ADC output is stored in ADC1BUF0 as this point
    AD1CON1bits.SAMP = 0; //Stop sampling
    AD1CON1bits.ADON = 0; //Turn off ADC, ADC value stored in ADC1BUF0;
    return (ADCvalue); //returns 10 bit ADC output stored in ADC1BIF0 to calling function
}
