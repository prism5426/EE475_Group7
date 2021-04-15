/*
 * Ultrasonic.h
 *
 *  Created on: Apr 14, 2021
 *      Author: liyij
 */

#ifndef INC_ULTRASONIC_H_
#define INC_ULTRASONIC_H_

#include <stdlib.h>
#include <stdbool.h>
#include "hcsr04_sensor.h"

#define trigPin 1
#define echoPin 0

typedef struct ultrasonicData {
	float* distance;
	hcsr04_data_t* HCSR04_data;
} ultrasonicData;

void ultrasonicTask(void*);

#endif /* INC_ULTRASONIC_H_ */
