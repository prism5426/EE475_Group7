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
#include "HCSR04.h"

typedef struct ultrasonicData {
	double* distancesInCm[4];
} ultrasonicData;

void ultrasonicTask(void*);

#endif /* INC_ULTRASONIC_H_ */
