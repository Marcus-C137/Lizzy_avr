#include "NTCthermistor.h"
#include <avr/io.h>
#include <stdlib.h>
#include <stdio.h>
#include <AnalogPin.h>
#include <util/delay.h>
#include <mUSART.h>

NTCthermistor thstor1(ADC_MUXPOS_AIN4_gc, 10000, 3950, 10000);
NTCthermistor thstor2(ADC_MUXPOS_AIN5_gc, 10000, 3950, 10000);
NTCthermistor thstor3(ADC_MUXPOS_AIN6_gc, 10000, 3950, 10000);
NTCthermistor thstor4(ADC_MUXPOS_AIN7_gc, 10000, 3950, 10000);
NTCthermistor thstorArray[4] = {thstor1, thstor2, thstor3, thstor4};

NTCthermistor::NTCthermistor(adc_0_channel_t adcPin, uint16_t nomRes, uint16_t bCoef, uint16_t serialRes)
{
    _adcPin = adcPin;
    _nomRes = nomRes;
    _bCoef = bCoef;
    _serialRes = serialRes;
    connected = false;
    temp = 0;
}

void NTCthermistor::read(void)
{
    uint8_t i;
    uint16_t sample;
    float average = 0;

    // take N samples in a row, with a slight delay
    for (i=0; i< 5; i++)
    {
    sample = ADC_0_get_conversion(_adcPin);
    if (sample < 10){
        connected = false;
        return;
    }
    average += sample;
    _delay_ms(10);
    }
    average /= 5;

    // convert the value to resistance
    average = 1023 / average - 1;
    average = _serialRes * average;

    float steinhart;
    steinhart = average / _nomRes;              // (R/Ro)
    steinhart = log(steinhart);                 // ln(R/Ro)
    steinhart /= _bCoef;                        // 1/B * ln(R/Ro)
    steinhart += 1.0 / (25.0 + 273.15);         // + (1/To)
    steinhart = 1.0 / steinhart;                // Invert
    steinhart -= 273.15;                        // convert to C
    steinhart = (steinhart * (9.0/5.0)) + 32;
    temp = (int) (steinhart*10.0);
    if (temp > 15 && temp < 2000){
        connected = true;
    }else{
        connected=false;
    }

}
