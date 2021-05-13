#include "scheduler.h"

#include <stm32f4xx_hal.h>
#include <stdbool.h>

static SchedulerCallback task_callbacks[SCHED_TASK_COUNT];
static volatile bool task_requests[SCHED_TASK_COUNT] = {false};

void sched_enroll(SchedulableTask task, SchedulerCallback callback) {
	task_callbacks[task] = callback;
}

void sched_request(SchedulableTask task) {
	// Set the flag that the task needs to be executed.
	task_requests[task] = true;

	// Set the event register so that WFE won't sleep when it's not supposed to.
	__SEV();
}

void sched_execute() {
	// Iterate over all the tasks.
	for (int i = 0; i < SCHED_TASK_COUNT; i++) {
		if (task_requests[i]) {
			// Clear the request before running the task, in case the task schedules itself again.
			task_requests[i] = false;

			task_callbacks[i]();
		}
	}
}
