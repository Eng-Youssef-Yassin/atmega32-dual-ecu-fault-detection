/*
 * Motor.h
 *
 *  Created on: Nov 4, 2025
 *      Author: Youssef Yassin
 */

#ifndef MOTOR_H2_
#define MOTOR_H2_

#include "std_types.h"

/*******************************************************************************
 *                         Pin Configuration
 *******************************************************************************/

/* Direction inputs to H-bridge */
#define DC_MOTOR_IN1_PORT_ID2    PORTA_ID   /* IN1 pin: PB0 */
#define DC_MOTOR_IN1_PIN_ID2     2

#define DC_MOTOR_IN2_PORT_ID2    PORTA_ID   /* IN2 pin: PB1 */
#define DC_MOTOR_IN2_PIN_ID2     3

/* PWM Enable pin (OC0 output) is PB3 */
#define DC_MOTOR_ENABLE_PORT_ID2 PORTB_ID
#define DC_MOTOR_ENABLE_PIN_ID2  3  /* Connected to OC0 for PWM speed control */

/*******************************************************************************
 *                           Types
 *******************************************************************************/
typedef enum
{
    STOP2,      /* Motor off */
    CW2,        /* Clockwise rotation */
    ACW2        /* Anti-clockwise rotation */
} DcMotor_State2;

/*******************************************************************************
 *                      Function Prototypes
 *******************************************************************************/

/*
 * Initializes the DC motor:
 *   - Sets IN1, IN2, and ENABLE pins as outputs
 *   - Stops the motor initially
 */
void DcMotor_Init2(void);

/*
 * Controls the motor rotation and speed.
 * Parameters:
 *   state : STOP, CW, or ACW (direction of rotation)
 *
 */
void DcMotor_Rotate2(DcMotor_State2 state);

#endif /* MOTOR_H_ */
