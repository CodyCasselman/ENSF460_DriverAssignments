/*
 * File:   Assignment4Main.c
 * Authors: Cody C, Evan M, Keeryn J
 *
 */

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
//Defines to improve readability
#define BUTTON_1 PORTAbits.RA2
#define BUTTON_2 PORTBbits.RB4
#define BUTTON_3 PORTAbits.RA4
#define LED LATBbits.LATB8
#define TIMER1 T1CONbits.TON
#define TIMER2 T2CONbits.TON
#define TIMER3 T3CONbits.TON

#include "xc.h"
#include "TimeDelay.h"
#include "IOs.h"
#include "clkChange.h"
#include "UART2.h"
#include "ADC.h"
#include <stdio.h>

//Define states
typedef enum { OFF, ON } State;
State mode = OFF;  //Initial state is off

uint16_t readingRate; //Rate of reading analog

//State conversions and reading analog flags
uint8_t startBuffer;
uint8_t PB2ActionFlag, PB3ActionFlag;
uint8_t blinking, brightnessFlashing;
uint8_t enterStateFlag, changeMode;
uint8_t beginRecordingFlag, recordingFlag;
uint8_t cyclesSinceRecording;

uint32_t adcValue; //Reading from analog input

uint16_t PB1History, PB2History, PB3History; //Previous states of buttons

uint16_t cycleTime; //Time for on/off durations
uint32_t intensity; //Brightness wanted for PWM
uint32_t highTime; //Percentage on in PWM
uint32_t lowTime; //Percentage off in PWM
uint8_t led_high; //State of LED for PWM flipping
uint32_t blink_time, recording_time, seconds_spent_recording;
uint8_t adc_flag = 0;

int main(void) {
    newClk(8); //set the clock speed
    
    // INITIALIZATION OF UART, IO, ADC and TIMERS   
    InitUART2();
    IOInit();
    TimerInit();    
    ADCInit();
    
    //Default analog read rate
    readingRate = 10;
    
    //delay_us(50, 3); //50ms buffer delay
    //delay_us(readingRate, 2); //Default to reading rate for analog
    //delay_us(255, 1); //set recording timer to 0.255s (Gives a 0.02s buffer to collect all data)
    
    //Buttons have not been interacted with
    PB1History = 0;
    PB2History = 0;
    PB3History = 0;
    
    startBuffer = 0;
    brightnessFlashing = 0; //Controls the PWM flashing
    blinking = 0; //Controls the 0.5s interval flashing
    PB2ActionFlag = 0;
    PB3ActionFlag = 0;
    enterStateFlag = 1; //Enter a state initially
    changeMode = 0;
    recordingFlag = 0;
    recording_time = 0;
    seconds_spent_recording = 0;
    cyclesSinceRecording = 0;
    
    cycleTime = 10000; //Pulse period is overall time section
    intensity = 50;
    led_high = 0; //0 for off
    blink_time = 0;
    LED = 0;
    TIMER2 = 0;
    /*
    while(1) {
        if(!startBuffer) {
            switch(mode) {
                case ON:
                    //STATE EXIT LOGIC
                    if (exitStateFlag){
                        mode = OFF;
                        enterStateFlag = 1;
                        exitStateFlag = 0;
                        TIMER2 = 0;
                        continue;
                        //
                    }
                    //BELOW LOGIC ALLOWS FOR CONSTANT BRIGHTNESS CHANGING

                    //Map the ADC value (0-1023) to brightness percentage (0-100)
                    Disp2String("\r");
                    Disp2String("AdcValue: ");
                    adcValue = do_ADC();
                    Disp2Dec(adcValue);
                    Disp2String("  Brightness: ");
                    
                    brightness = (adcValue * 100) / 1023;
                    Disp2Dec(brightness);
                    //Calculate highTime and lowTime based on brightness
                    highTime = (brightness * cycleTime) / 100;
                    lowTime = cycleTime - highTime;
                    Disp2String("  High Time: ");
                    Disp2Dec(highTime);
                    Disp2String("  Low Time: ");
                    
                    Disp2Dec(lowTime);
                    /*
                    Disp2String("\r\n");
                    Disp2String("LED state: ");
                    Disp2Dec(ledState);
                    Disp2String("   LED Value: ");
                    Disp2Dec(LED);
                    Disp2String("\033[1A");
                    
       
                    //Blink the LED at the calculated times
                    if (!ledState) {
                        delay_ms(highTime, 2);  // LED ON for highTime
                        LED = 1;
                        ledState = 1;
                    } else {
                        delay_ms(lowTime, 2);   // LED OFF for lowTime
                        LED = 0;
                        ledState = 0;
                    }
                    
                    //BELOW LOGIC CONTROLS PB2 INPUTS
                    /*
                    //PB2 pressed, flash for 0.5s intervals
                    if (PB2ActionFlag) {
                        flashing = 1; //Allow LED changing in TMR2
                        LED = 1; //Start with LED on
                    }
                    
                    //PB2 pressed again, stop 0.5s flashes
                    if (!PB2ActionFlag && flashing) {
                        flashing = 0;
                        LED = 1; //Back to on for PWM
                    }

                    //BELOW LOGIC CONTROLS PB3 INPUTS
                    
                    //PB3 pressed, begin reading
                    if(beginRecordingFlag && PB3ActionFlag){
                        Disp2String("BEGIN\n");
                        beginRecordingFlag = 0;
                        recordingFlag = 1;
                        
                        TIMER1 = 1; //begin counting up to ~1 seconds
                        TMR2 = 0; //Reset TMR2 for sending out adc readings
                    }
                    if(recordingFlag && PB3ActionFlag){
                        adcValue = do_ADC();
                        Disp2Dec(adcValue);
                        Disp2String("\n");
                    }
                    
                    //STATE ENTRY LOGIC
                    if (enterStateFlag){
                        enterStateFlag = 0;
                        LED = 1; //Start with LED on
                        TMR2 = 0;
                        TIMER2 = 1; //Start timer for PWM, always going for PWM
                        //brightnessFlashing = 1; //TMR2 PWM control on
                        //Turn on LED, reset TMR2, and start TIMER2 for PWM
                        adcValue = do_ADC();
                        brightness = (adcValue * 100) / 1023;
                        highTime = (brightness * cycleTime) / 100;
                        delay_ms(highTime, 2);  // LED ON for highTime
                        ledState = 1;
                    }

                    break;
                case OFF:
                    //STATE EXIT LOGIC
                    if (exitStateFlag){
                        mode = ON;
                        enterStateFlag = 1;
                        exitStateFlag = 0;
                        TIMER2 = 0;
                        continue;
                        //Stop flashing timer on transition
                    }
                    
                    //BELOW LOGIC CONTROLS PB2 INPUTS
                    /*
                    //PB2 pressed
                    if (PB2ActionFlag) {
                        flashing = 1; //Allow LED changing in TMR2
                        LED = 1; //Start with LED on
                        TIMER2 = 1; //Start flashing timer
                    }
                    
                    //PB2 pressed again
                    if (!PB2ActionFlag && flashing) {
                        flashing = 0;
                        LED = 0;
                        TIMER2 = 0;
                        TMR2 = 0;
                    }
                     
                    //Idle(); //Default to Idle if off
                    
                    //STATE ENTRY LOGIC
                    if (enterStateFlag){
                        enterStateFlag = 0;
                        LED = 0; //Start with LED off
                        TMR2 = 0;
                        delay_ms(500, 2); //0.5s flashing
                        brightnessFlashing = 0; //TMR2 PWM control off
                        //Turn off LED, reset TMR2 and stop PWM and 0.5s flashing
                    }   

                    break;
            }
        }
        
        Idle(); //Processor spends most of its time in Idle
    }
    */
    
    TIMER1 = 0;
    set_timerX_us(10000, 1);
    set_timerX_us(cycleTime, 2);
    set_timerX_us(100000, 3);
    while (1) {
        //Map the ADC value (0-1023) to brightness percentage (0-100)
        
        switch (mode) {
            case ON:
                TIMER1 = 1;
                TIMER2 = 1;
                if (adc_flag) {

                    adc_flag = 0;
                    adcValue = do_ADC();
                    intensity = (adcValue * 100) / 1023;
                    if (intensity >= 100)
                        intensity = 99;
                    if (intensity <= 2)
                        intensity = 0;
                    highTime = (intensity * cycleTime) / 100;
                    lowTime = cycleTime - highTime;
                    if (recordingFlag) {
                        Disp2String("\rIntensity: "); 
                        Disp2Dec(intensity);
                        Disp2String("   ADC value: "); 
                        Disp2Dec(adcValue);
                        Disp2String("\n"); 
                    }
                }
                
                if (changeMode)
                {
                    changeMode = 0;
                    mode = OFF;
                    continue;
                }
                break;
                
            case OFF:
                
                
                if (blinking)   //If off, but blinking is turned on, the timers should be on
                {
                    TIMER1 = 1;
                    TIMER2 = 1;
                    PR2 = (uint16_t)(500000 / 16);
                } else {        //If off, and blinking is off, both timers should also be off
                    TIMER1 = 0;
                    TIMER2 = 0;
                    TMR2 = 0;
                    LED = 0;
                }
                
                if (changeMode)
                {
                    changeMode = 0;
                    mode = ON;
                    continue;
                }
                break;
        }
        Idle();
    }
    return 0;
}

void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void){
    IFS0bits.T1IF = 0; //Clear flag
    if (recordingFlag)
    {
        recording_time += 10000;
        if (recording_time > 1000000) {
            seconds_spent_recording += 1;
            recording_time = 0;
        }
        
        if (seconds_spent_recording > 59){
            recordingFlag = 0;
            seconds_spent_recording = 0;
        }
    }
    
    if (blinking)
        blink_time += 10000;
    
    
    adc_flag = 1;
}

void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void){ 
    //Timer 2 is our flash timer
    IFS0bits.T2IF = 0; //Clear flag
    led_high ^= 1;
    
    if (mode == OFF && blinking) 
        LED ^= 1;
        
    //If the blinking is turned off or if time has reached above 500000 micro seconds (half a second)
    //Allow the led to glow at the right brightness 
    if (mode == ON) {
        if (blink_time > 500000 || blinking == 0) 
        {
            if (intensity == 0) LED = 0;    //LED is off if at minimum brightness
            else if (intensity == 99) LED = 1;    //LED is on if at maximum brightness

            //If brightness is between 1 and 99 then high and low time is used
            else if (!led_high) {       
                LED = 1;
                PR2 = (uint16_t)highTime / 16;
            } else {
                LED = 0;
                PR2 = (uint16_t)lowTime / 16;
            }

            if (blink_time > 1000000){ //If the full 1 second cycle of the blink has passed reset it to zero
                blink_time = 0;
                LED = 0;
            }
        } else {
            LED = 0;
            //intensity = 0;
        }
    }
}

void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void){
    //Timer 3 is our buffer timer
    IFS0bits.T3IF = 0; //Clear flag
    TIMER3 = 0; //Turn timer off
    startBuffer = 0; //Reset buffer flag
    //isolate the most recent button states. See if it was on and then off
    if((PB1History & 0b11) == 0b10)
    {
        changeMode = 1; //CHANGE STATE
        PB1History = 0; //Reset PB1History
    }
    if((PB2History & 0b11) == 0b10)
    {
        blinking ^= 1; //Activate or deactivate blinking
        PB2History = 0; //Reset PB2History
    }
    if((PB3History & 0b11) == 0b10)
    {
        if (mode == ON) {
            recordingFlag = 1; //Start reading on PB3 press        
        }
        PB3History = 0; //Reset PB2History
    }
}

void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void){
    //CN interrupt should start the buffer and collect inputs
    IFS1bits.CNIF = 0; //Clear flag 
    //Left shift button history, concatenate with current button value
    PB1History = (PB1History << 1) | (BUTTON_1 ^ 1);
    PB2History = (PB2History << 1) | (BUTTON_2 ^ 1);
    PB3History = (PB3History << 1) | (BUTTON_3 ^ 1);
    //Start buffer timer and reset timer value
    startBuffer = 1;
    TMR3 = 0;
    TIMER3 = 1;
}
