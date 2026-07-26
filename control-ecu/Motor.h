/*
 * Motor.h
 *
 *  Created on: Nov 4, 2025
 *      Author: Youssef Yassin
 */

#ifndef MOTOR_H_
#define MOTOR_H_

#include "std_types.h"

/*******************************************************************************
 *                         Pin Configuration
 *******************************************************************************/

/* Direction inputs to H-bridge */
#define DC_MOTOR_IN1_PORT_ID    PORTA_ID   /* IN1 pin: PB0 */
#define DC_MOTOR_IN1_PIN_ID     5

#define DC_MOTOR_IN2_PORT_ID    PORTA_ID   /* IN2 pin: PB1 */
#define DC_MOTOR_IN2_PIN_ID     6

/* PWM Enable pin (OC0 output) is PB3 */
#define DC_MOTOR_ENABLE_PORT_ID PORTB_ID
#define DC_MOTOR_ENABLE_PIN_ID  3   /* Connected to OC0 for PWM speed control */

/*******************************************************************************
 *                           Types
 *******************************************************************************/
typedef enum
{
    STOP,      /* Motor off */
    CW,        /* Clockwise rotation */
    ACW        /* Anti-clockwise rotation */
} DcMotor_State;

/*******************************************************************************
 *                      Function Prototypes
 *******************************************************************************/

/*
 * Initializes the DC motor:
 *   - Sets IN1, IN2, and ENABLE pins as outputs
 *   - Stops the motor initially
 */
void DcMotor_Init(void);

/*
 * Controls the motor rotation and speed.
 * Parameters:
 *   state : STOP, CW, or ACW (direction of rotation)
 *   speed : Duty cycle percentage (0–100 %) for speed
 */
void DcMotor_Rotate(DcMotor_State state);

#endif /* MOTOR_H_ */
