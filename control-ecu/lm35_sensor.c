/******************************************************************************
 *
 * Module: Temperature Sensor
 *
 * File Name: lm35_sensor.c
 *
 * Description: source file for the LM35 Temperature Sensor driver
 *
 * Author: Mohamed Tarek
 *
 *******************************************************************************/

#include <util/delay.h> /* For the delay functions */
#include "lm35_sensor.h"
#include "adc.h"

/*
 * Function responsible for calculate the temperature from the ADC digital value.
 */

void LM35_init(void)
{

	ADC_ConfigType adcConfig = { INTERNAL_REF, PRESCALER_64 };

	ADC_init(&adcConfig);
}



uint8 LM35_getTemperature(void)
{



	double temp_value = 0;
	uint16 g_adcResult =0;

	/* Read ADC channel where the temperature sensor is connected */
	g_adcResult = ADC_readChannel(SENSOR_CHANNEL_ID);

	/* Calculate the temperature from the ADC value*/
	temp_value = (double)(((double)g_adcResult*SENSOR_MAX_TEMPERATURE*ADC_REF_VOLT_VALUE)/(ADC_MAXIMUM_VALUE*SENSOR_MAX_VOLT_VALUE));

	return (uint8) temp_value;
}

