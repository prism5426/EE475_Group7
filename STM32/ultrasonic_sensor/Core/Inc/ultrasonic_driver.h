#pragma once

#include<stdint.h>

#include "results.h"

typedef enum {
	ULTRASONIC_STATE_TRIGGERED,
	ULTRASONIC_STATE_PULSE_BEGAN,
	ULTRASONIC_STATE_PULSE_ENDED,
	ULTRASONIC_STATE_OVERCAPTURED,
	ULTRASONIC_STATE_TIMED_OUT,
	ULTRASONIC_STATE_ERROR,
} UltrasonicState;

typedef struct UltrasonicMeasurement UltrasonicMeasurement;
typedef struct UltrasonicHardware UltrasonicHardware;

typedef void (*UltrasonicCallback)(const volatile UltrasonicMeasurement *sensor);

struct UltrasonicMeasurement {
	/// Callback function used to signal when state changes. This is called from an interrupt handler, so set whatever flags you need to and get out quick.
	UltrasonicCallback callback;

	/// Opaque pointer passed to callback.
	void *user_data;

	UltrasonicState state;
	int pulse_begin;
	int pulse_end;

	/// Internal.
	const UltrasonicHardware *hw;
};

/* @brief Initializes the ultrasonic driver.
 */
void ultrasonic_driver_init();

/* @brief Triggers an ultrasonic sensor.
 * @param sensor_index Which sensor to trigger. Ranges between [0, 6] inclusive.
 * @param measurement Structure used to exchange measurement results. The callback and user_data fields should be initialized by the caller.
 */
ResultCode ultrasonic_driver_trigger_sensor(int sensor_index, UltrasonicMeasurement *measurement);
