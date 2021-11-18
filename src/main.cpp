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
#include "../lib/CurrentSensor/CurrentSensor.h"
#include "../lib/i2c/myI2c.h"

int main(void)
{
    USART0_init();
    heatinit();
    ADC_0_init();
    i2csetup();
    sei();
    
    while (1) 
    {
        heatLoop();

    }
}


