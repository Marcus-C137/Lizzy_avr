#ifndef mANALOGPIN
#define mANALOGPIN

#include <avr/io.h>

typedef void (*adc_irq_cb_t)(void);

/** Datatype for the result of the ADC conversion */
typedef uint16_t adc_result_t;

//* Analog channel selection */
typedef ADC_MUXPOS_t adc_0_channel_t;

int8_t ADC_0_init();

void ADC_0_start_conversion(adc_0_channel_t channel);

bool ADC_0_is_conversion_done();

adc_result_t ADC_0_get_conversion_result(void);

adc_result_t ADC_0_get_conversion(adc_0_channel_t channel);

uint8_t ADC_0_get_resolution();

#endif