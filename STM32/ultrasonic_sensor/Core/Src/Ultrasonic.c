#include "Ultrasonic.h"
#include <stdio.h>

// sensor pin configuration
GPIO_TypeDef* triggerPorts[4] = {Trigger0_GPIO_Port, Trigger1_GPIO_Port, Trigger2_GPIO_Port, Trigger3_GPIO_Port};
uint16_t triggerPins[4] = {Trigger0_Pin, Trigger1_Pin, Trigger2_Pin, Trigger3_Pin};
GPIO_TypeDef* echoPorts[4] = {Echo0_GPIO_Port, Echo1_GPIO_Port, Echo2_GPIO_Port, Echo3_GPIO_Port};
uint16_t echoPins[4] = {Echo0_Pin, Echo1_Pin, Echo2_Pin, Echo3_Pin};

void updateDistance(double* distancesInCm[4]) {
	for (int i = 0; i < 4; i++) {
		*distancesInCm[i] = measureDistance(triggerPorts[i], triggerPins[i], echoPorts[i], echoPins[i]);
	}
}

void ultrasonicTask(void* usData) {
	ultrasonicData* data = (ultrasonicData*) usData;
	updateDistance(data->distancesInCm);
}
