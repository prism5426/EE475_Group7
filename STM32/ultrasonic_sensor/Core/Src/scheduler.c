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

	// Trigger a PendSV interrupt.
	SCB->ICSR|= SCB_ICSR_PENDSVSET_Msk;
}

void sched_execute() {
	// Clear the interrupt flag first. If any more tasks are requested while we are executing,
	// then this flag will be reset and we will reenter the PendSV handler.
	SCB->ICSR|= SCB_ICSR_PENDSVCLR_Msk;

	// Iterate over all the tasks.
	for (int i = 0; i < SCHED_TASK_COUNT; i++) {
		if (task_requests[i]) {
			// Clear the request before running the task, in case the task schedules itself again.
			task_requests[i] = false;

			task_callbacks[i]();
		}
	}
}
