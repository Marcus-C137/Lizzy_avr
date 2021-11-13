#ifndef HEATSETUP
#define HEATSETUP
#include "NTCthermistor.h"

void heatinit(void);
void heatLoop(void);
void heatOut(NTCthermistor *thstor, int heatport);

#endif 