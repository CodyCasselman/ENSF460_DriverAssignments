/*
 * File:   ADC.c
 * Author: codyc
 *
 * Created on October 28, 2024, 1:19 PM
 */


#include "xc.h"

void ADCInit() {
    TRISAbits.TRISA3 = 1; //Potentiometer input
    
    AD1CON1bits.ADSIDL = 1; //Don't continue in Idle mode
    AD1CON1bits.FORM = 00; //Sets data output format (Integer)
    AD1CON1bits.SSRC = 0b111; //Sets to auto conversion (Sets based on internal counter)
    AD1CON1bits.ASAM = 0; //Need to set SAMP to start sampling (1 for input)
    AD1CON2bits.VCFG = 0b000; 
    AD1CON2bits.CSCNA = 0; //Don't scan inputs
    AD1CON3bits.SAMC = 0b11111;
    AD1CON3bits.ADRC = 1; //Use internal RC clock
    AD1CON3bits.ADCS = 0b11111; //Slowest sampling rate
    AD1CHSbits.CH0NA = 0;
    AD1CHSbits.CH0SA = 0b0101; //Channel 0 positive input is AN5
}

uint16_t do_ADC(void)
{
    uint16_t ADCvalue ; // 16 bit register used to hold ADC converted digital output ADC1BUF0
    
    AD1CON1bits.ADON = 1; //Turns ADC on
    AD1CON1bits.SAMP = 1; //Start Sampling, Conversion starts automatically after SSRC and SAMC settings
    
    
    while(AD1CON1bits.DONE==0) {}
    ADCvalue = ADC1BUF0; // ADC output is stored in ADC1BUF0 as this point
    AD1CON1bits.SAMP=0; //Stop sampling
    AD1CON1bits.ADON=0; //Turn off ADC, ADC value stored in ADC1BUF0;
    return (ADCvalue); //returns 10 bit ADC output stored in ADC1BIF0 to calling function
}
