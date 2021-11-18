#include "Heat.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <mUSART.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "NTCthermistor.h"

volatile int setTemps[4] = {0};
volatile int gains[4] = {0};
volatile bool portsOn[4] = {0};

void heatinit(void){ //Heating Output

    //set as output
    PORTB.DIRSET = PIN5_bm;
    PORTC.DIRSET = PIN0_bm;
    PORTF.DIRSET = PIN4_bm;
    PORTF.DIRSET = PIN5_bm;
    //start as low
    PORTB.OUTCLR = PIN5_bm;
    PORTC.OUTCLR = PIN0_bm;
    PORTF.OUTCLR = PIN4_bm;
    PORTF.OUTCLR = PIN5_bm;

    //Interrupt pin
    PORTB.DIRCLR = PIN4_bm;
    PORTB.PIN4CTRL = PORT_PULLUPEN_bm | PORT_ISC_RISING_gc;
    
    TCA0_SINGLE_CTRLA |= 0b00000011; //Prescaler 2 & Enable
    TCA0_SINGLE_CTRLESET &= ~(1 << 0); //Timer A count up

    uint16_t startTop = 5130; //~ 2ms
    uint16_t startTime = 0;
    TCB0_CCMP = startTop; TCB1_CCMP = startTop; TCB2_CCMP = startTop; TCB3_CCMP = startTop;
    TCB0_CNT = startTime; TCB1_CNT = startTime; TCB2_CNT = startTime; TCB3_CNT = startTime;
    // Use CLK_PER as counter
    TCB0_CTRLA &= ~(3 << 2);  
    TCB1_CTRLA &= ~(3 << 2);  
    TCB2_CTRLA &= ~(3 << 2);  
    TCB3_CTRLA &= ~(3 << 2);  
    // Turn off restart with TCA
    TCB0_CTRLB &= ~(1 << 4);
    TCB1_CTRLB &= ~(1 << 4);
    TCB2_CTRLB &= ~(1 << 4);
    TCB3_CTRLB &= ~(1 << 4);
    // pin is intially zero
    TCB0_CTRLB &= ~(1 << TCB_CCMPINIT_bp); 
    TCB1_CTRLB &= ~(1 << TCB_CCMPINIT_bp); 
    TCB2_CTRLB &= ~(1 << TCB_CCMPINIT_bp); 
    TCB3_CTRLB &= ~(1 << TCB_CCMPINIT_bp); 
    // Output disable
    TCB0_CTRLB &= ~(1 << TCB_CCMPEN_bp); 
    TCB1_CTRLB &= ~(1 << TCB_CCMPEN_bp); 
    TCB2_CTRLB &= ~(1 << TCB_CCMPEN_bp); 
    TCB3_CTRLB &= ~(1 << TCB_CCMPEN_bp); 
    //set to periodic
    TCB0_CTRLB &= ~(0x7); 
    TCB1_CTRLB &= ~(0x7); 
    TCB2_CTRLB &= ~(0x7); 
    TCB3_CTRLB &= ~(0x7);
    //disable event
    TCB0_EVCTRL = 0;
    TCB1_EVCTRL = 0;
    TCB2_EVCTRL = 0;
    TCB3_EVCTRL = 0;

    //enable interrupt
    TCB0.INTCTRL |= (1 << TCB_CAPT_bp);
    TCB1.INTCTRL |= (1 << TCB_CAPT_bp);
    TCB2.INTCTRL |= (1 << TCB_CAPT_bp);
    TCB3.INTCTRL |= (1 << TCB_CAPT_bp);

    //Turn enable on
    TCB0_CTRLA |= (1 << 0); 
    TCB1_CTRLA |= (1 << 0); 
    TCB2_CTRLA |= (1 << 0); 
    TCB3_CTRLA |= (1 << 0);

}

void heatLoop(void){
    heatOut(&curSens1, &thstor1, 1);
    heatOut(&curSens2, &thstor2, 2);
    heatOut(&curSens3, &thstor3, 3);
    heatOut(&curSens4, &thstor4, 4);
}

void heatOut(ACS712 *curSens, NTCthermistor *thstor, int heatPort){

    int portOn = 0;
    uint16_t timeOn = 100;
    int setTemp = 900;
    thstor->read();
    curSens->read(); 
    int tempDiff = setTemp - thstor->temp;
    if(tempDiff > 200) tempDiff = 200;
    if (tempDiff > 0 && thstor->connected && !curSens->overcurrent){
        timeOn = ((tempDiff/10) * -1000) + 20100;
        portOn = 1;
    }
    if (heatPort == 1) {TCB0_CCMP = timeOn; TCB0.INTCTRL = portOn;} 
    if (heatPort == 2) {TCB1_CCMP = timeOn; TCB1.INTCTRL = portOn;}
    if (heatPort == 3) {TCB2_CCMP = timeOn; TCB2.INTCTRL = portOn;}
    if (heatPort == 4) {TCB3_CCMP = timeOn; TCB3.INTCTRL = portOn;}

}

ISR(PORTB_PORT_vect)
{
    PORTB.OUTCLR = PIN5_bm;
    PORTC.OUTCLR = PIN0_bm;
    PORTF.OUTCLR = PIN4_bm;
    PORTF.OUTCLR = PIN5_bm;
    TCB0_CNT = 0; 
    TCB1_CNT = 0; 
    TCB2_CNT = 0; 
    TCB3_CNT = 0;
    PORTB.INTFLAGS = PORT_INT4_bm;

}

ISR(TCB0_INT_vect)
{
    PORTB.OUTSET = PIN5_bm;
    //dMsg("1");
    TCB0.INTFLAGS = 1;
}

ISR(TCB1_INT_vect)
{
    PORTC.OUTSET = PIN0_bm;
    //dMsg("1");
    TCB1.INTFLAGS = 1;
}

ISR(TCB2_INT_vect)
{
    PORTF.OUTSET = PIN4_bm;
    //dMsg("1");
    TCB2.INTFLAGS = 1;
}

ISR(TCB3_INT_vect)
{
    PORTF.OUTSET = PIN5_bm;
    //dMsg("1");
    TCB3.INTFLAGS = 1;
}