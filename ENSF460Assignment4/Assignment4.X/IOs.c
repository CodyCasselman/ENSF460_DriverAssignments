#include "IOs.h"
#include "TimeDelay.h"
#include "xc.h"

#define BUTTON_1 PORTAbits.RA2
#define BUTTON_2 PORTBbits.RB4
#define BUTTON_3 PORTAbits.RA4
#define LED LATBbits.LATB8
#define TIMER2 T2CONbits.TON
#define TIMER3 T3CONbits.TON

void IOInit() {
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

void TimerInit() {
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
    //PR2 = 3125; //100ms delay at 500kHz clock freq.
    
    TMR3 = 0;
    TIMER3 = 0;
}


uint8_t IOCheck(){
    uint8_t PB_status;
    PB_status = BUTTON_1;
    return PB_status;
}

