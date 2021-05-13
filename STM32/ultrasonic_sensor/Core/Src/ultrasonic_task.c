#include "ultrasonic_task.h"

#include<stm32f4xx_hal.h>
#include<stdio.h>

#include "util.h"
#include "ultrasonic_driver.h"
#include "scheduler.h"

static const UltrasonicScheduleEntry *ultrasonic_schedule = NULL;
static UltrasonicTaskCallback ultrasonic_callback = NULL;

static int pulse_widths[7];

static int ultrasonic_schedule_index = -1;
static volatile int ultrasonic_sensors_active;

static UltrasonicMeasurement measurement_a;
static UltrasonicMeasurement measurement_b;

static void ultrasonic_begin_schedule_entry(int index);
static void ultrasonic_measurement_updated(const volatile UltrasonicMeasurement *measurement);
static void ultrasonic_task_run();

void ultrasonic_task_init(const UltrasonicScheduleEntry *new_schedule, UltrasonicTaskCallback new_callback) {
	// Initialize the ultrasonic driver.
	ultrasonic_driver_init();

	// Initialize schedule and callback
	ultrasonic_schedule = new_schedule;
	ultrasonic_callback = new_callback;

	// Enroll our task in the scheduler.
	sched_enroll(SCHED_TASK_ULTRASONIC, &ultrasonic_task_run);
}

ResultCode ultrasonic_task_poll() {
	// Check that we're not already polling.
	if (ultrasonic_schedule_index != -1) {
		return R_ULTRASONIC_TASK_BUSY;
	}

	// Start the first item on our schedule.
	ultrasonic_begin_schedule_entry(0);

	return R_OK;
}

static void ultrasonic_task_run() {
	// Check if we're done with the schedule entry we're on.
	if (ultrasonic_sensors_active == 0) {
		ultrasonic_schedule_index++;

		// Check if we've reached the end of the schedule.
		if (ultrasonic_schedule[ultrasonic_schedule_index].sensor_a == -1 && ultrasonic_schedule[ultrasonic_schedule_index].sensor_b == -1) {
			// Set schedule index to -1 so we don't think we're busy anymore (in case the callback calls ultrasonic_task_poll).
			ultrasonic_schedule_index = -1;

			// Invoke the callback to return our measurements.
			ultrasonic_callback(pulse_widths);
		} else {
			// Start the next schedule entry
			ultrasonic_begin_schedule_entry(ultrasonic_schedule_index);
		}
	}
}

static void ultrasonic_begin_schedule_entry(int index) {
	const UltrasonicScheduleEntry *schedule_entry = &ultrasonic_schedule[index];

	measurement_a.callback = &ultrasonic_measurement_updated;
	measurement_a.user_data = &pulse_widths[schedule_entry->sensor_a]; // sensor_a might be negeative one, but it's ok to produce an invalid pointer if you don't use it.

	measurement_b.callback = &ultrasonic_measurement_updated;
	measurement_b.user_data = &pulse_widths[schedule_entry->sensor_b];

	ultrasonic_schedule_index = index;
	ultrasonic_sensors_active = 0;

	// Figure out how many sensors are active during this schedule entry and initialize pulse widths.
	if (schedule_entry->sensor_a != -1) {
		ultrasonic_sensors_active++;
		pulse_widths[schedule_entry->sensor_a] = -1;
	}

	if (schedule_entry->sensor_b != -1) {
		ultrasonic_sensors_active++;
		pulse_widths[schedule_entry->sensor_b] = -1;
	}

	// Need to have ultrasonic_sensors_active stable before this point, since after this we're liable to be interrupted.

	if (schedule_entry->sensor_a != -1) {
		if (ultrasonic_driver_trigger_sensor(schedule_entry->sensor_a, &measurement_a) != R_OK) {
			// TODO: abort
		}
	}

	if (schedule_entry->sensor_b != -1) {
		if (ultrasonic_driver_trigger_sensor(schedule_entry->sensor_b, &measurement_b) != R_OK) {
			// TODO: abort
		}
	}
}

static void ultrasonic_measurement_updated(const volatile UltrasonicMeasurement *measurement) {
	int *pulse_width = (int*) measurement->user_data;

	switch (measurement->state) {
		case ULTRASONIC_STATE_TRIGGERED:
			// Callback shouldn't be called for this state.
			// TODO: abort?
			break;
		case ULTRASONIC_STATE_PULSE_BEGAN:
			// Don't care.
			break;

		case ULTRASONIC_STATE_PULSE_ENDED:
			// Got a good reading. Store the pulse width.
			*pulse_width = measurement->pulse_end - measurement->pulse_begin;

			// fall-through
		case ULTRASONIC_STATE_OVERCAPTURED:
			// fall-through
		case ULTRASONIC_STATE_TIMED_OUT:
			// fall-through
		case ULTRASONIC_STATE_ERROR:
			// All of these states mean that the measurement is complete. If we didn't have a good reading, it's ok because
			// pulse_width was initialized to -1 when we began this schedule entry.

			if (--ultrasonic_sensors_active == 0) {
				// Schedule the ultrasonic task.
				sched_request(SCHED_TASK_ULTRASONIC);
			}
	}
}
