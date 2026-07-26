/*
 * pwm.c
 *
 *  Created on: Nov 4, 2025
 *      Author: Youssef Yassin
 */

#include"pwm.h"
#include"common_macros.h"
#include<avr/io.h>

/* timer0 regiesters
   REG TCCR0
   FOC0=0 PWM MODE
    WGM01:WGM00=11 FAST PWM
    COM01=1  COM00=0 non inverting mode
    CS02=0 CS01=1 CS00=1 prescaler 64



 */

void PWM_Timer0_Start(uint8 duty_cycle)
{
	SET_BIT(DDRB,3); //pinb3 output OC0

	TCCR0=(1<<WGM00) |(1<<WGM01) |(1<<COM01) |(1<<CS01) |(1<<CS00);

	OCR0=(uint8)((uint32)(duty_cycle*255)/100);

}


