/*
 * timer.c
 *
 *  Created on: Nov 4, 2025
 *      Author: Youssef Yassin
 */

#include "timer.h"
#include "common_macros.h" /* To use the macros like SET_BIT, CLEAR_BIT */
#include <avr/io.h>        /* To use Timer Registers */
#include <avr/interrupt.h> /* For Timer ISRs */

/*******************************************************************************
 *                           Global Variables                                  *
 *******************************************************************************/

/* Global variables to hold the addresses of callback functions */
static volatile void (*g_callBackPtr0)(void) = NULL_PTR;
static volatile void (*g_callBackPtr1)(void) = NULL_PTR;
static volatile void (*g_callBackPtr2)(void) = NULL_PTR;

/*******************************************************************************
 *                       Interrupt Service Routines                            *
 *******************************************************************************/

ISR(TIMER0_OVF_vect)
{
	if (g_callBackPtr0 != NULL_PTR)
		(*g_callBackPtr0)();
}

ISR(TIMER0_COMP_vect)
{
	if (g_callBackPtr0 != NULL_PTR)
		(*g_callBackPtr0)();
}

ISR(TIMER1_OVF_vect)
{
	if (g_callBackPtr1 != NULL_PTR)
		(*g_callBackPtr1)();
}

ISR(TIMER1_COMPA_vect)
{
	if (g_callBackPtr1 != NULL_PTR)
		(*g_callBackPtr1)();
}

ISR(TIMER2_OVF_vect)
{
	if (g_callBackPtr2 != NULL_PTR)
		(*g_callBackPtr2)();
}

ISR(TIMER2_COMP_vect)
{
	if (g_callBackPtr2 != NULL_PTR)
		(*g_callBackPtr2)();
}

/*******************************************************************************
 *                      Functions Definitions                                  *
 *******************************************************************************/

/*
 * Description:
 * Initialize the specified timer with the given configuration.
 */
void Timer_init(const Timer_ConfigType *Config_Ptr)
{
	switch (Config_Ptr->timer_ID)
	{
	case TIMER0_ID:
		switch (Config_Ptr->timer_Mode)
		{
		case TIMER_MODE_NORMAL:
			/*
			 * TCCR0 Configuration:
			 * FOC0 = 1 (Non-PWM)
			 * WGM00 = 0, WGM01 = 0 (Normal mode)
			 * CS02:0 = Clock Select (prescaler)
			 */
			TCCR0 = (1 << FOC0) | (Config_Ptr->timer_Clock & 0x07);
			TCNT0 = (uint8)Config_Ptr->timer_InitialValue;
			SET_BIT(TIMSK, TOIE0); /* Enable Overflow Interrupt */
			break;

		case TIMER_MODE_CTC:
			/*
			 * TCCR0 Configuration:
			 * FOC0 = 1 (Non-PWM)
			 * WGM00 = 0, WGM01 = 1 (CTC mode)
			 * CS02:0 = Clock Select (prescaler)
			 */
			TCCR0 = (1 << FOC0) | (1 << WGM01) | (Config_Ptr->timer_Clock & 0x07);
			TCNT0 = (uint8)Config_Ptr->timer_InitialValue;
			OCR0 = (uint8)Config_Ptr->timer_CompareMatchValue;
			SET_BIT(TIMSK, OCIE0); /* Enable Compare Match Interrupt */
			break;
		}
		break;

	case TIMER1_ID:
		switch (Config_Ptr->timer_Mode)
		{
		case TIMER_MODE_NORMAL:
			/*
			 * TCCR1A/B Configuration:
			 * FOC1A/B = 1 (Non-PWM)
			 * WGM13:0 = 0000 (Normal mode)
			 */
			TCCR1A = (1 << FOC1A) | (1 << FOC1B);
			TCCR1B = (Config_Ptr->timer_Clock & 0x07);
			TCNT1 = Config_Ptr->timer_InitialValue;
			SET_BIT(TIMSK, TOIE1); /* Enable Overflow Interrupt */
			break;

		case TIMER_MODE_CTC:
			/*
			 * TCCR1A/B Configuration:
			 * FOC1A/B = 1 (Non-PWM)
			 * WGM12 = 1 (CTC mode using OCR1A)
			 */
			TCCR1A = (1 << FOC1A) | (1 << FOC1B);
			TCCR1B = (1 << WGM12) | (Config_Ptr->timer_Clock & 0x07);
			TCNT1 = Config_Ptr->timer_InitialValue;
			OCR1A = Config_Ptr->timer_CompareMatchValue;
			SET_BIT(TIMSK, OCIE1A); /* Enable Compare Match Interrupt */
			break;
		}
		break;

	case TIMER2_ID:
		switch (Config_Ptr->timer_Mode)
		{
		case TIMER_MODE_NORMAL:
			/*
			 * TCCR2 Configuration:
			 * FOC2 = 1 (Non-PWM)
			 * WGM21:0 = 00 (Normal)
			 */
			TCCR2 = (1 << FOC2) | (Config_Ptr->timer_Clock & 0x07);
			TCNT2 = (uint8)Config_Ptr->timer_InitialValue;
			SET_BIT(TIMSK, TOIE2); /* Enable Overflow Interrupt */
			break;

		case TIMER_MODE_CTC:
			/*
			 * TCCR2 Configuration:
			 * FOC2 = 1 (Non-PWM)
			 * WGM21 = 1 (CTC)
			 */
			TCCR2 = (1 << FOC2) | (1 << WGM21) | (Config_Ptr->timer_Clock & 0x07);
			TCNT2 = (uint8)Config_Ptr->timer_InitialValue;
			OCR2 = (uint8)Config_Ptr->timer_CompareMatchValue;
			SET_BIT(TIMSK, OCIE2); /* Enable Compare Match Interrupt */
			break;
		}
		break;
	}
}

/*
 * Description:
 * Deinitialize (stop) the specified timer and clear its registers.
 */
void Timer_deInit(Timer_ID_Type timer_ID)
{
	switch (timer_ID)
	{
	case TIMER0_ID:
		TCCR0 = 0;
		TCNT0 = 0;
		OCR0 = 0;
		CLEAR_BIT(TIMSK, TOIE0);
		CLEAR_BIT(TIMSK, OCIE0);
		break;

	case TIMER1_ID:
		TCCR1A = 0;
		TCCR1B = 0;
		TCNT1 = 0;
		OCR1A = 0;
		CLEAR_BIT(TIMSK, TOIE1);
		CLEAR_BIT(TIMSK, OCIE1A);
		break;

	case TIMER2_ID:
		TCCR2 = 0;
		TCNT2 = 0;
		OCR2 = 0;
		CLEAR_BIT(TIMSK, TOIE2);
		CLEAR_BIT(TIMSK, OCIE2);
		break;
	}
}

/*
 * Description:
 * Set the callback function address for the corresponding timer.
 */
void Timer_setCallBack(void (*a_ptr)(void), Timer_ID_Type timer_ID)
{
	switch (timer_ID)
	{
	case TIMER0_ID:
		g_callBackPtr0 = a_ptr;
		break;
	case TIMER1_ID:
		g_callBackPtr1 = a_ptr;
		break;
	case TIMER2_ID:
		g_callBackPtr2 = a_ptr;
		break;
	}
}
