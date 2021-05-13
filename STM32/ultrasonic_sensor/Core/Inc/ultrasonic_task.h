#pragma once

#include "results.h"

#define ULTRASONIC_NUM_SENSORS 7

typedef struct {
	// Negative one means no sensor should be polled at this point in the schedule.
	int sensor_a;
	int sensor_b;
} UltrasonicScheduleEntry;

typedef void (*UltrasonicTaskCallback)(int *pulse_widths);

/* @brief Initializes the ultrasonic task.
 * @param schedule Pointer to an array of UltrasonicSchedule entries that determines the order in which sensors will be polled (as well as which sensors are polled at the same time). This array is terminated by an entry where both sensor indices are negative one.
 * @param callback Callback function that is invoked when the end of the schedule is reached and all sensors have been polled.
 */
void ultrasonic_task_init(const UltrasonicScheduleEntry *schedule, UltrasonicTaskCallback callback);

/* @brief Starts polling all of the sensors.
 */
ResultCode ultrasonic_task_poll();
