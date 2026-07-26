/*
 * ultrasonic.c
 *
 *  Created on: Nov 4, 2025
 *      Author: Youssef Yassin
 */

#include"std_types.h"
#include"ultrasonic.h"
#include"gpio.h"
#include<avr/io.h>
#include <util/delay.h>
#include"icu.h"

float distance=0;
uint8 g_edgeCount = 0;
uint16 g_timeHigh = 0;


ICU_ConfigType ICU_Configurations = {F_CPU_8,RAISING};


void Ultrasonic_init(void)
{
	ICU_init(&ICU_Configurations);

	ICU_setCallBack(Ultrasonic_edgeProcessing);

	GPIO_setupPinDirection(TRIGGER_PORT, TRIGGER_PIN, PIN_OUTPUT);


}

void Ultrasonic_Trigger(void)
{
	GPIO_writePin(TRIGGER_PORT, TRIGGER_PIN,LOGIC_HIGH);
	_delay_us(10);
	GPIO_writePin(TRIGGER_PORT, TRIGGER_PIN,LOGIC_LOW);


}



uint16 Ultrasonic_readDistance(void)
{
	uint32 period = 0;
	uint32 timeout = 0;

	Ultrasonic_Trigger();

	while((g_edgeCount < 2) && (timeout < 60000))
	{
	    _delay_us(1);
	    timeout++;
	}

		g_edgeCount = 0;

		/* calculate the period */
		period = g_timeHigh;

		distance=period/58.8;


	return distance;
}

void Ultrasonic_edgeProcessing(void)
{
    g_edgeCount++;
    if(g_edgeCount == 1)
    {
        ICU_clearTimerValue();                 // start timing at first rising edge
        ICU_setEdgeDetectionType(FALLING);     // next edge: falling
    }
    else if(g_edgeCount == 2)
    {
        g_timeHigh = ICU_getInputCaptureValue(); // pulse width
        ICU_setEdgeDetectionType(RAISING);      // prepare for next reading
    }
}




