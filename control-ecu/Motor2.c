/*
 * Motor.c
 *
 *  Created on: Nov 4, 2025
 *      Author: Youssef Yassin
 */


#include "Motor2.h"
#include "gpio.h"
#include "PWM.h"
#include "avr/io.h"




/*
 Configure IN1, IN2, and Enable pins as outputs and stop motor.
*/
void DcMotor_Init2(void)
{
    /* Configure direction pins */
    GPIO_setupPinDirection(DC_MOTOR_IN1_PORT_ID2, DC_MOTOR_IN1_PIN_ID2, PIN_OUTPUT);
    GPIO_setupPinDirection(DC_MOTOR_IN2_PORT_ID2, DC_MOTOR_IN2_PIN_ID2, PIN_OUTPUT);

    /* Configure enable pin (PB3) as output for PWM (OC0) */
    GPIO_setupPinDirection(DC_MOTOR_ENABLE_PORT_ID2, DC_MOTOR_ENABLE_PIN_ID2, PIN_OUTPUT);

    /* Ensure the motor is initially stopped */
    GPIO_writePin(DC_MOTOR_IN1_PORT_ID2, DC_MOTOR_IN1_PIN_ID2, 0);
    GPIO_writePin(DC_MOTOR_IN2_PORT_ID2, DC_MOTOR_IN2_PIN_ID2, 0);

    /* Start PWM with 0% duty to keep motor off until commanded */
    PWM_Timer0_Start(0);
}

/*---------------------------------------------------------------------------
 *    Drive the motor in the requested direction and set speed.
 *    state – STOP, CW, or ACW
 *
 *--------------------------------------------------------------------------*/
void DcMotor_Rotate2(DcMotor_State2 state)
{


	 switch (state)
	    {
	        case STOP2:
	            GPIO_writePin(DC_MOTOR_IN1_PORT_ID2, DC_MOTOR_IN1_PIN_ID2, 0);
	            GPIO_writePin(DC_MOTOR_IN2_PORT_ID2, DC_MOTOR_IN2_PIN_ID2, 0);
	            PWM_Timer0_Start(0);
	            break;

	        case CW2:
	            GPIO_writePin(DC_MOTOR_IN1_PORT_ID2, DC_MOTOR_IN1_PIN_ID2, 1);
	            GPIO_writePin(DC_MOTOR_IN2_PORT_ID2, DC_MOTOR_IN2_PIN_ID2, 0);
	            PWM_Timer0_Start(100);
	            break;

	        case ACW2:
	            GPIO_writePin(DC_MOTOR_IN1_PORT_ID2, DC_MOTOR_IN1_PIN_ID2, 0);
	            GPIO_writePin(DC_MOTOR_IN2_PORT_ID2, DC_MOTOR_IN2_PIN_ID2, 1);
	            PWM_Timer0_Start(100);
	            break;
	    }
	}
