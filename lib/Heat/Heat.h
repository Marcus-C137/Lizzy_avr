#ifndef HEATSETUP
#define HEATSETUP
#include "NTCthermistor.h"
#include "CurrentSensor.h"

void heatinit(void);
void heatLoop(void);
void heatOut(ACS712 *curSens, NTCthermistor *thstor, int heatport);
extern volatile int setTemps[4]; // temp * 10
extern volatile int gains[4]; 
extern volatile bool portsOn[4]; 

#endif 