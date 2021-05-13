#pragma once

#include<stdint.h>

typedef enum {
	ULTRASONIC_STATE_INITIAL,
	ULTRASONIC_STATE_TRIGGERED,
	ULTRASONIC_STATE_PULSE_BEGAN,
	ULTRASONIC_STATE_PULSE_ENDED,
	ULTRASONIC_STATE_OVERCAPTURED,
	ULTRASONIC_STATE_TIMED_OUT,
	ULTRASONIC_STATE_ERROR,
} UltrasonicState;

typedef struct {
	UltrasonicState state;
	uint32_t pulse_begin;
	uint32_t pulse_end;
} Ultrasonic;

extern volatile Ultrasonic ultrasonic_sensors[7];

void ultrasonic_task_init();
void ultrasonic_task_run();
