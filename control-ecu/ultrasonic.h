/*
 * ultrasonic.h
 *
 *  Created on: Nov 4, 2025
 *      Author: Youssef Yassin
 */

#ifndef ULTRASONIC_H_
#define ULTRASONIC_H_

#include "std_types.h"

#define TRIGGER_PORT PORTD_ID
#define TRIGGER_PIN PIN7_ID



void Ultrasonic_init(void);
void Ultrasonic_Trigger(void);
uint16 Ultrasonic_readDistance(void);
void Ultrasonic_edgeProcessing(void) ;



#endif /* ULTRASONIC_H_ */
