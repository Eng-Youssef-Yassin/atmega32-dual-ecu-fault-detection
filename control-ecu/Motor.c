/*
 * Motor.c
 *
 *  Created on: Nov 4, 2025
 *      Author: Youssef Yassin
 */

#include "Motor.h"
#include "gpio.h"
#include "PWM.h"
#include "avr/io.h"


/*
 Configure IN1, IN2, and Enable pins as outputs and stop motor.
*/
void DcMotor_Init(void)
{
    /* Configure direction pins */
    GPIO_setupPinDirection(DC_MOTOR_IN1_PORT_ID, DC_MOTOR_IN1_PIN_ID, PIN_OUTPUT);
    GPIO_setupPinDirection(DC_MOTOR_IN2_PORT_ID, DC_MOTOR_IN2_PIN_ID, PIN_OUTPUT);

    /* Configure enable pin (PB3) as output for PWM (OC0) */
    GPIO_setupPinDirection(DC_MOTOR_ENABLE_PORT_ID, DC_MOTOR_ENABLE_PIN_ID, PIN_OUTPUT);

    /* Ensure the motor is initially stopped */
    GPIO_writePin(DC_MOTOR_IN1_PORT_ID, DC_MOTOR_IN1_PIN_ID, 0);
    GPIO_writePin(DC_MOTOR_IN2_PORT_ID, DC_MOTOR_IN2_PIN_ID, 0);
    //GPIO_writePin(DC_MOTOR_ENABLE_PORT_ID, DC_MOTOR_ENABLE_PIN_ID, 1);

    /* Start PWM with 0% duty to keep motor off until commanded */
    PWM_Timer0_Start(0);
}

/*---------------------------------------------------------------------------
 *    Drive the motor in the requested direction and set speed.
 *    state – STOP, CW, or ACW
 *    speed – Duty cycle percentage (0–100) for PWM speed control
 *--------------------------------------------------------------------------*/
void DcMotor_Rotate(DcMotor_State state)
{



    switch (state)
    {
        case STOP:
            /* Both direction pins LOW → motor stopped */
            GPIO_writePin(DC_MOTOR_IN1_PORT_ID, DC_MOTOR_IN1_PIN_ID, 0);
            GPIO_writePin(DC_MOTOR_IN2_PORT_ID, DC_MOTOR_IN2_PIN_ID, 0);
            PWM_Timer0_Start(0);    // 0% duty cycle
            break;

        case CW:
            /* IN1 HIGH, IN2 LOW → Clockwise rotation */
            GPIO_writePin(DC_MOTOR_IN1_PORT_ID, DC_MOTOR_IN1_PIN_ID, 1);
            GPIO_writePin(DC_MOTOR_IN2_PORT_ID, DC_MOTOR_IN2_PIN_ID, 0);
            PWM_Timer0_Start(100); // Set speed with duty cycle
            break;

        case ACW:
            /* IN1 LOW, IN2 HIGH → Anti-clockwise rotation */
            GPIO_writePin(DC_MOTOR_IN1_PORT_ID, DC_MOTOR_IN1_PIN_ID, 0);
            GPIO_writePin(DC_MOTOR_IN2_PORT_ID, DC_MOTOR_IN2_PIN_ID, 1);
            PWM_Timer0_Start(100);
            break;
    }
}
