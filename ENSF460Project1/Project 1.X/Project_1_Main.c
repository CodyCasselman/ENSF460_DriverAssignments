/*
 * File:   assignment3_main.c
 * Author: 
 *
 * Created on October 3, 2024, 6:00 PM
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
#define BUTTON_1 PORTAbits.RA2
#define BUTTON_2 PORTBbits.RB4
#define BUTTON_3 PORTAbits.RA4
#define LED LATBbits.LATB8
#define TIMER2 T2CONbits.TON
#define TIMER3 T3CONbits.TON

#include "xc.h"
#include "TimeDelay.h"
#include "IOs.h"
#include "clkChange.h"
#include "state.h"
#include "UART2.h"
#include <stdio.h>

uint16_t PBX_event;
state_t state;
uint8_t startBuffer;
uint8_t PB3_value;
uint8_t PB_status;
uint8_t previous_PB_status;
uint8_t timer_finished;

uint8_t secs_increment;
uint8_t mins_increment;

uint8_t pb1_down;
uint8_t pb2_down;
uint8_t pb3_down;
uint8_t pb3_previous;

uint8_t secs_incrementing;
uint8_t mins_incrementing;

uint8_t secs_hold_time;
uint8_t mins_hold_time;
uint8_t clear_hold_time;

uint8_t state_flag;
uint8_t countdown;

uint16_t seconds;
uint16_t minutes;

uint8_t str[2];
int main(void) {
    AD1PCFG = 0xFFFF; /* keep this line as it sets I/O pins that can also be analog to be digital */
    newClk(8);
    IPC4bits.CNIP = 6;
    IFS1bits.CNIF = 0;
    IEC1bits.CNIE = 1;
        
    
    // INITIALIZATION OF UART, IO and TIMERS   
    InitUART2();
    IOinit();
    Timerinit();
    
    
    // VALUES
    // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    secs_increment = 1; //Amount to increment seconds by
    mins_increment = 1; //Amount to increment minutes by
    secs_hold_time = 0; //Time PB2 has been held
    mins_hold_time = 0; //Time PB1 has been held
    clear_hold_time = 0;//Time PB3 has been held
    seconds = 0;    
    minutes = 0;
    // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    
    
    // FLAGS
    // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    state = SET;        //State variable holding, set, clear, finish, or count
    PBX_event = 0;      //Whether input should be taken
    PB_status = 0;      //Current status of the buttons, 3 digit binary number
    timer_finished = 0; //Whether the timer has finish    
    startBuffer = 0;    //Flag to start the input buffer timer
    
    //PB flags 0 = OFF, 1 = ON
    pb1_down = 0;
    pb2_down = 0;
    pb3_down = 0;
    pb3_previous = 0;
    
    secs_incrementing = 0;  //Should seconds increment
    mins_incrementing = 0;  //Should minutes increment
    
    state_flag = 0; //Has a state been entered
    countdown = 0;  //Should the timer be counting down
    // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    
    Disp2String("SET 00m : 00s          \r");
    
    
    while(1) {
        if (startBuffer)        //If the CN interrupt has turned on the input buffer
        {
            TMR2 = 0;           //Reset the timer
            delay_ms(10, 2);    //Set the input buffer time to 10 milliseconds
            TIMER2 = 1;         //Turn it on
             
        }
        if(PBX_event)   //Once timer 2 sends back that the input buffer is over, read the inputs
        {
            TIMER2 = 0; 
            //record the status of each button, set delay accordingly
            PB_status = IOCheck();          //Get inputs, returns a 3 digit binary number with each digit representing a button
            pb1_down = PB_status & 0b001;   //Isolate PB1 state
            pb2_down = PB_status & 0b010;   //Isolate PB2 state
            pb3_previous = pb3_down;        //store previous PB3 input
            pb3_down = PB_status & 0b100;   //Get new PB3 input 
            
            PBX_event = 0;  //Reset the flag to only take one input
        }
        
        if (!startBuffer){
            switch (state) {
                //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                case SET:
                    // EXIT CONDITIONS
                    // ============================
                    if (!pb3_down && pb3_previous)  // IF PB3 pressed then released move to COUNT
                    {
                        state = CNT;        //Set new state
                        state_flag = 0;     //Clear flag saying a state has been entered                        
                        TIMER3 = 0;         //Reset timer 3 to ensure count controls it as it sees fit
                        TMR3 = 0;
                        continue;           //Continue to immediately go to new state
                    }
                    
                    if (clear_hold_time == 3)   // IF PB3 pressed and held for 5 seconds move to CLEAR
                    {
                        state = CLR;        //Set new state
                        TIMER3 = 0;         //Turn off timer since clear does not use it
                        TMR3 = 0;
                        clear_hold_time = 0;//Reset the time the clear button has been held for
                        state_flag = 0;     //Clear flag saying a state has been entered
                        LED = 0;
                        continue;           //Continue to immediately go to new state
                    }
                    // ============================
                    
                    // ENTERING STATE FUNCTIONALITY
                    if (!state_flag)
                        enter_state(SET);
                    
                    // MAIN STATE FUNCTIONALITY
                    if (pb1_down || pb2_down){
                        TIMER3 = 1; //If either button to increment time is pressed, turn on the timer
                        if (pb1_down && !mins_incrementing) //If PB1 is pressed and the minutes are not incrementing...
                        {
                            mins_increment = 1;     //Reset the increment value
                            mins_hold_time = 0;     //Reset the time the button has been held
                            mins_incrementing = 1;  //Tell the timer that minutes should increment
                            minutes += 1;           //Add 1 to minutes for the initial button press
                        } else if (!pb1_down) 
                            mins_incrementing = 0;  //If the button is released tell the timer to stop incrementing minutes
                           
                        //See minutes functionality above, works the same
                        if (pb2_down && !secs_incrementing) 
                        {
                            secs_increment = 1;
                            secs_hold_time = 0;
                            secs_incrementing = 1;
                            seconds += 1;
                        } else if (!pb2_down) 
                            secs_incrementing = 0; 
                    } 
                    else    //If neither button is pressed...
                    {
                        if (pb3_down)   //Check if the clear button is being held
                            TIMER3 = 1; //If so don't stop the timer as its counting clear time up
                        else {
                            TIMER3 = 0; //Otherwise fully reset the clock
                            TMR3 = 0;
                        }
                        mins_incrementing = 0;  //Tell the timer to stop incrementing both seconds and minutes
                        secs_incrementing = 0;
                    }
                    disp_time();    //Display the time
                    break;
                    
                // END CASE SET
                //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                    
                //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                case CLR:      
                    // ENTERING STATE FUNCTIONALITY
                    if (!state_flag)
                    {
                        enter_state(CLR);
                    }      
                    
                    // EXIT CONDITION
                    // ============================
                    if (pb1_down || pb2_down)   //If either PB1 or 2 are pressed, go to SET
                    {
                        state = SET;
                        state_flag = 0;
                        continue;
                    }
                    // ============================
                    break;
                // END CASE CLEAR
                //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                   
                //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                case FIN:
                    // ENTERING STATE FUNCTIONALITY
                    if (!state_flag)
                    {
                        enter_state(FIN);
                    }
                    
                    // EXIT CONDITION
                    // ============================
                    if (pb1_down || pb2_down)   //If either PB1 or 2 are pressed, clear the alarm tag, turn off LED, then go to set
                    {
                        Disp2String("\033[13C");
                        Disp2String("         \r");
                        state = SET;
                        state_flag = 0;
                        LED = 0;
                        continue;
                    }
                    // ============================
                    break;
                // END CASE FINISH
                //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<    
                    
                //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                case CNT:
                    // EXIT CONDITIONS
                    // ============================
                    if (clear_hold_time == 3)   // Same as sets clear exit condition
                    {
                        state = CLR;
                        TIMER3 = 0;
                        TMR3 = 0;
                        countdown = 0;      //Tell timer to stop counting down
                        clear_hold_time = 0;
                        state_flag = 0;
                        LED = 0;    //Turn LED off in case it was on for the last flash
                        continue;
                    }
                    
                    if (countdown == 0 && (pb1_down || pb2_down))   // If while the countdown is paused PB1 or 2 are pressed, go to SET
                    {
                        state = SET;
                        TIMER3 = 0;
                        TMR3 = 0;
                        countdown = 0;      //Tell timer to stop counting down
                        clear_hold_time = 0;
                        state_flag = 0;
                        LED = 0;    //Turn LED off in case it was on for the last flash
                        continue;
                    }
                    
                    if (timer_finished) //If the timer sets the flag that both seconds and minutes == 0, go to FINISH
                    {
                        state = FIN;
                        state_flag = 0;
                        timer_finished = 0; //Clear the timer finished flag
                        TIMER3 = 0;
                        TMR3 = 0;
                        continue;
                    }
                    // ============================
                    
                    // ENTERING STATE FUNCTIONALITY 
                    if (!state_flag)
                    {
                        enter_state(CNT);                        
                    }                      
                    
                    // MAIN FUNCTIONALITY
                    if (pb3_down == 0 && pb3_previous)  //If PB3 pressed then released..
                    {
                        countdown ^= 1;     //Flip countdown flag
                        LED = 0;
                        clear_hold_time = 0;//Reset clear hold time
                        pb3_previous = 0;   //Say that PB3 is now released so this doesn't happen twice
                    }
                    
                    disp_time();
                    break;
                // END CASE COUNT
                //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                default:
                    break;
            };
        }
        Idle();
    }
        
    return 0;
}

// Timer 2 interrupt subroutine
// This is only being used for the input delay buffer
// When triggered it allows for entry into IOCheck to get the current inputs just after the buttons were pressed.
void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void){
    //Don't forget to clear the timer 2 interrupt flag!
    IFS0bits.T2IF = 0;
    PBX_event = 1;      //Allow entry into IOCheck
    startBuffer = 0;    //Clear the flag for starting the input buffer
}

// Timer 3 interrupt subroutine
// This is used as the recurring increment timer
// Used for when holding PB1 or 2 for incrementing seconds and minutes automatically 
// Used for when holding PB3 to count the seconds its been held to move to clear
// Used for while inside count, while countdown is turned on to decrement minutes and seconds one second at a time, in time
void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void){
    //Don't forget to clear the timer 3 interrupt flag!
    IFS0bits.T3IF = 0;
    
    // SET FUNCTIONALITY
    if (state == SET)
    {
        if (pb3_down)   //Count clear time
            clear_hold_time += 1; 
        if (mins_incrementing && pb1_down) {  //If PB1 pressed, and minutes should be incrementing...
            minutes += mins_increment;  //Increment minutes by the current amount (1 or 5)
            mins_hold_time += 1;        //Increment the time its been held for
            if (minutes >= 60)          //Overflow to 0 at 60
                minutes = 0;
            //If the increment is 1, the button has been held for 3 seconds, and minutes currently ends in 0 or 5...
            if (mins_increment == 1 && mins_hold_time >= 3 && (minutes % 5 == 0))
                mins_increment *= 5;    //Set increment to 5
        }

        //See minutes functionality above, seconds works the same
        if (secs_incrementing && pb2_down){  
            seconds += secs_increment;
            secs_hold_time += 1;
            if (seconds >= 60)
                seconds = 0;
            if (secs_increment == 1 && secs_hold_time >= 3 && (seconds % 5 == 0))
                secs_increment *= 5;
        }
    }
    
    //COUNT FUNCTIONALITY
    if (state == CNT) {
        if (pb3_down)   //Count clear time
            clear_hold_time += 1; 
        if (countdown) {    //If counting down...
            LED ^= 1;   //Flash the LED on and off
            if (seconds == 0)
            {                        
                if (minutes == 0)
                {
                   timer_finished = 1;  // If 00:00 is the time, set the timer finished tag to 1
                } else {
                    minutes--;  //If only seconds is 0 then minutes goes down and seconds goes to 59
                    seconds = 59;
                }
            } else seconds--;   //If neither are 0, just seconds goes down

            
        }
    }
    
    TMR3 = 0;
}

// CN interrupt subroutine
// When any button is pressed and input buffer begins, waiting 10ms before allowing entry into IOCheck to grab the current inputs
// Improves input accuracy and helps avoid bouncing
void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void){
    IFS1bits.CNIF = 0; //Clear flag 
    startBuffer = 1;
}

// Display the current value of minutes and seconds with UART
// moves the cursor to the position of each part and overwrites it with the new value
void disp_time()
{
    Disp2String("\033[4C");
    if (minutes < 10)
        Disp2String("0");
    sprintf(str, "%d", minutes);
    Disp2String(str);
    Disp2String("\r");

    Disp2String("\033[10C");
    if (seconds < 10)
        Disp2String("0");
    sprintf(str, "%d", seconds);
    Disp2String(str);
    Disp2String("\r");
}

// Entering state functionality
void enter_state(state_t s)
{
    switch (s) {
        case SET:
            delay_ms(1000, 3);      //Set the delay of timer 3 for incrementing to 1 second
            Disp2String("SET\r");   //Add the title
            state_flag = 1;         //Set the state flag to 1 indicating the state has been entered
            break;
            
        case CLR:
            Disp2String("CLR 00m : 00s \r");    //Clear the current time and reset minutes and seconds 
            minutes = 0;    
            seconds = 0;
            break;
            
        case FIN:
            Disp2String("FIN\r");       //Print the title, and -- ALARM to the screen
            Disp2String("\033[13C");
            Disp2String(" -- ALARM\r");
            state_flag = 1;
            LED = 1;    //Turn LED on
            break;
            
        case CNT:
            delay_ms(1000, 3);      //Set timer 3 to increment at 1 second
            Disp2String("CNT\r");   //Print the title
            state_flag = 1;
            countdown = 0;          //Turn countdown off to begin
            TIMER3 = 1;             //Turn the timer on
            break;
        default:
            break;
    }  
}