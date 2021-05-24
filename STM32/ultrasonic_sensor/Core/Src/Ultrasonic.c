#include "Ultrasonic.h"

void updateDistance(float* distance, float sensor_distance) {
	*distance = sensor_distance;

}

void ultrasonicTask(void* usData) {
	ultrasonicData* data = (ultrasonicData*) usData;
	HCSR04_GetInfo(data->HCSR04_data);
	updateDistance(data->distance, (data->HCSR04_data)->distance_cm);
}
