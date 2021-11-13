
#include <AnalogPin.h>

int8_t ADC_0_init()
{


	ADC0.CTRLC = ADC_PRESC_DIV4_gc      /* CLK_PER divided by 4 */
	             | ADC_REFSEL_VDDREF_gc /* Internal reference */
	             | 0 << ADC_SAMPCAP_bp; /* Sample Capacitance Selection: disabled */

	ADC0.CTRLA = 1 << ADC_ENABLE_bp     /* ADC Enable: enabled */
	             | 0 << ADC_FREERUN_bp  /* ADC Freerun mode: disabled */
	             | ADC_RESSEL_10BIT_gc  /* 10-bit mode */
	             | 0 << ADC_RUNSTBY_bp; /* Run standby mode: disabled */

	return 0;
}


/**
 * \brief Start a conversion on ADC_0
 *
 * \param[in] channel The ADC channel to start conversion on
 *
 * \return Nothing
 */
void ADC_0_start_conversion(adc_0_channel_t channel)
{
	ADC0.MUXPOS  = channel;
	ADC0.COMMAND = ADC_STCONV_bm;
}

/**
 * \brief Check if the ADC conversion is done
 *
 * \return The status of ADC converison done check
 * \retval true The ADC conversion is done
 * \retval false The ADC converison is not done
 */
bool ADC_0_is_conversion_done()
{
	return (ADC0.INTFLAGS & ADC_RESRDY_bm);
}

/**
 * \brief Read a conversion result from ADC_0
 *
 * \return Conversion result read from the ADC_0 ADC module
 */
adc_result_t ADC_0_get_conversion_result(void)
{
	return (ADC0.RES);
}

/**
 * \brief Start a conversion, wait until ready, and return the conversion result
 *
 * \return Conversion result read from the ADC_0 ADC module
 */
adc_result_t ADC_0_get_conversion(adc_0_channel_t channel)
{
	adc_result_t res;

	ADC_0_start_conversion(channel);
	while (!ADC_0_is_conversion_done())
		;
	res = ADC_0_get_conversion_result();
	ADC0.INTFLAGS |= ADC_RESRDY_bm;
	return res;
}

/**
 * \brief Return the number of bits in the ADC conversion result
 *
 * \return The number of bits in the ADC conversion result
 */
uint8_t ADC_0_get_resolution()
{
	return (ADC0.CTRLA & ADC_RESSEL_bm) ? 8 : 10;
}