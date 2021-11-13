#include "../lib/USART/mUSART.h"
#include "../lib/Heat/Heat.h"
#include "../lib/AnalogPin/AnalogPin.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../lib/NTCthermistor/NTCthermistor.h"


int main(void)
{
    USART0_init();
    heatinit();
    ADC_0_init();
    
    sei();
    
    while (1) 
    {
        heatLoop();

    }
}


