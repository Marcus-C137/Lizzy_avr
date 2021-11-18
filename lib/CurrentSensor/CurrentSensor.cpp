#include "CurrentSensor.h"
#include <avr/io.h>
#include <AnalogPin.h>
#include <mUSART.h>

ACS712 curSens1(ADC_MUXPOS_AIN0_gc);
ACS712 curSens2(ADC_MUXPOS_AIN1_gc);
ACS712 curSens3(ADC_MUXPOS_AIN2_gc);
ACS712 curSens4(ADC_MUXPOS_AIN3_gc);


ACS712::ACS712(adc_0_channel_t adcPin){

    _adcPin = adcPin;
    overcurrent = false;
}

void ACS712::read(){

    TCA0.SINGLE.CNT = 0;
    uint16_t maxSample = 400; // 1.65v - 0.0 Amps
    while(TCA0.SINGLE.CNT < 4000){
        uint16_t sample = ADC_0_get_conversion(_adcPin);
        if (sample > maxSample) maxSample = sample;
    }
    current = maxSample;
    if (maxSample > maxCur){
        overcurrent = true;
    }

}