#ifndef NTCtherm
#define NTCtherm
#include "stdlib.h"
#include <stdint.h>
#include <avr/io.h>
#include <AnalogPin.h>

class NTCthermistor
{
    private:
        adc_0_channel_t _adcPin;
        uint16_t _nomRes; 
        uint16_t _bCoef;
        uint16_t _serialRes;

    public:
        NTCthermistor(adc_0_channel_t adcPin, uint16_t nomRes, uint16_t bCoef, uint16_t serialRes);
        int temp;
        bool connected;
        void read(void);
    
};

extern NTCthermistor thstor1;
extern NTCthermistor thstor2;
extern NTCthermistor thstor3;
extern NTCthermistor thstor4;
extern NTCthermistor thstorArray[4];

void setUpADC(void);

#endif