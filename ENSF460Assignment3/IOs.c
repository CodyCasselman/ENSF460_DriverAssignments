#include "IOs.h"
#include "TimeDelay.h"
#include "xc.h"

#define BUTTON_1 PORTAbits.RA2
#define BUTTON_2 PORTBbits.RB4
#define BUTTON_3 PORTAbits.RA4
#define LED LATBbits.LATB8
#define TIMER2 T2CONbits.TON
#define TIMER3 T3CONbits.TON

void IOinit() {
    /* Let's set up some I/O */
    TRISBbits.TRISB8 = 0;
    
    TRISAbits.TRISA4 = 1;
    CNPU1bits.CN0PUE = 1;
    CNEN1bits.CN0IE = 1;
    
    TRISBbits.TRISB4 = 1;
    CNPU1bits.CN1PUE = 1;
    CNEN1bits.CN1IE = 1;
    
    TRISAbits.TRISA2 = 1;
    CNPU2bits.CN30PUE = 1;
    CNEN2bits.CN30IE = 1;
}

void Timerinit() {
    T2CONbits.T32 = 0; //16-Bit timers
    
    T3CONbits.TCKPS = 1;   //Prescaler for Timer 3 set to 1:8.
    T3CONbits.TCS = 0;     //Timer 2 clock source is the internal clock (Fcy).
    T3CONbits.TSIDL = 0;   //Timer 2 continues running in idle mode.
    IPC2bits.T3IP = 2;     //Interrupt priority for Timer 2 set to 2.
    IFS0bits.T3IF = 0;     //Clear Timer 2 interrupt flag.
    IEC0bits.T3IE = 1;     //Enable Timer 2 interrupt.
    
    T2CONbits.TCKPS = 1;   //Prescaler for Timer 2 set to 1:8.
    T2CONbits.TCS = 0;     //Timer 2 clock source is the internal clock (Fcy).
    T2CONbits.TSIDL = 0;   //Timer 2 continues running in idle mode.
    IPC1bits.T2IP = 2;     //Interrupt priority for Timer 2 set to 2.
    IFS0bits.T2IF = 0;     //Clear Timer 2 interrupt flag.
    IEC0bits.T2IE = 1;     //Enable Timer 2 interrupt.
    
    TMR2 = 0;
    TIMER2 = 0;
    PR2 = 3125; //100ms delay at 500kHz clock freq.
    
    TMR3 = 0;
    TIMER3 = 0;
}

void IOCheck(){
    uint16_t delay_time = 0;
    //PB_status contains a 3-bit value showing the states of each LED
    // 1 for ON, 0 for OFF
    uint8_t PB_status = (BUTTON_1 << 2 | BUTTON_2 << 1 | BUTTON_3) ^ 0b111;
    uint8_t flashing = 1;
    switch(PB_status){
        case 0b000:
            //turn LED off
            flashing = 0;
            LED = 0;
            Disp2String("Nothing Pressed                                 \r");
            break;
        case 0b100:
            LED = 1;
            delay_time = 500;
            Disp2String("PB1 Pressed                                  \r");
            break;
        case 0b010:
            LED = 1;
            delay_time = 1000;
            Disp2String("PB2 Pressed                                  \r");
            break;
        case 0b001:
            LED = 1;
            delay_time = 4000;
            Disp2String("PB3 Pressed                                  \r");
            break;
        default:
            flashing = 0;
            LED = 1;
            //turn LED on
            //potentially this is too many switch cases (what if there's a case where they add a 4th button)
            switch(PB_status){
                case 0b011:
                    Disp2String("PB2 and PB3 Pressed                                  \r");
                    break;
                case 0b101:
                    Disp2String("PB1 and PB3 Pressed                                  \r");
                    break;
                case 0b110:
                    Disp2String("PB1 and PB2 Pressed                                  \r");
                    break;
                case 0b111:
                    Disp2String("All PBs Pressed                       \r");
                    break;
                default:
                    break;
            }
    }
    if(flashing){
        delay_ms(delay_time);
        TIMER3 = 1;
    }
    else{
        TIMER3 = 0;
    }
    /*
        if (BUTTON_1 + BUTTON_2 + BUTTON_3 <= 2) 
            //if one or more buttons are pressed
        {
            if(BUTTON_1 + BUTTON_2 + BUTTON_3 == 0){ //if all 3 are pressed
                LED = 1;
            } 
            else if (BUTTON_1 + BUTTON_2 + BUTTON_3 == 1) { //Any 2 pressed
                LED = 1;
            }       
            else if (BUTTON_1 == 0){ //0.5 sec delay
                delay_time = 500;
            } 
            else if (BUTTON_2 == 0) { //1 sec delay
                delay_time = 1000;
            }
            else if (BUTTON_3 == 0) { // 4 sec delay
                delay_time = 4000;  
            }
            delay_ms(delay_time); //change timer values
            TIMER3 = 1; //Turn the blink timer on
        }
     * */
}

