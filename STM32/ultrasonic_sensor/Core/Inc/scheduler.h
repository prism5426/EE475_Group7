#pragma once

/* Each task that can be scheduled must have an assigned index here so that the scheduler knows at compile-time the maximum possible number of tasks that can exist. */
typedef enum {
	SCHED_TASK_ULTRASONIC,

	SCHED_TASK_COUNT,
} SchedulableTask;

typedef void (*SchedulerCallback)();

/* @brief Registers a schedulable task's callback with the scheduler. Must be called before requesting that the task be scheduled.
 * @param task Task index to enroll
 * @param callback Callback to call when executing the task
 */
void sched_enroll(SchedulableTask task, SchedulerCallback callback);

/* @brief Requests that a schedulable task be scheduled for execution.
 * @param task Task index to request
 */
void sched_request(SchedulableTask task);

/* @brief Runs any scheduled tasks. Should be called from PendSV interrupt handler. */
void sched_execute();
