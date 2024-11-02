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
    //Set the direction of RB4 (PB2) to input
    TRISBbits.TRISB4 = 1;
    CNPU1bits.CN1PUE = 1; //Enable the pull-up
    CNEN1bits.CN1IE = 1; //Enable the interrupt
    //Set the direction of RA2 (PB1) to input
    TRISAbits.TRISA2 = 1;
    CNPU2bits.CN30PUE = 1; //Enable the pull-up
    CNEN2bits.CN30IE = 1; //Enable the CN interrupt
    //Enable CN Interrupts for buttons and set their priority
    IPC4bits.CNIP = 6;
    IFS1bits.CNIF = 0;
    IEC1bits.CNIE = 1;
    //Set the direction of the potentiometer to be input
    TRISAbits.TRISA3 = 1; 
}




uint8_t IOCheck(){
    uint8_t PB_status;
    PB_status = BUTTON_1;
    return PB_status;
}

