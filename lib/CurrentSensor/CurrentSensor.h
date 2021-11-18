#ifndef CURRENTSENSOR
#define CURRENTSENSOR
#include <stdlib.h>
#include <AnalogPin.h> 

class ACS712
{
    private:
        adc_0_channel_t _adcPin;
        uint16_t maxCur = 667; //max current 5A

    public:
        ACS712(adc_0_channel_t adcPin);
        void read();
        bool overcurrent;
        uint16_t current = 512;

};

extern ACS712 curSens1;
extern ACS712 curSens2;
extern ACS712 curSens3;
extern ACS712 curSens4;

#endif