/*
 * Scheduler.h
 *
 *  Created on: November 12, 2024
 *      Author: Cody Jacobs
 *      Lab 5 - Scheduler Header
 */

#include <stdint.h>

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#define LED_TOGGLE_EVENT			(1 << 0)
#define LED_DELAY_EVENT				(1 << 1)
#define LED_POLL_EVENT				(1 << 2)
#define APP_DELAY_FLAG_EVENT 		(1 << 3)
#define DEVICE_ID_AND_TEMP_EVENT 	(1 << 4)
#define ROTATE_BLOCK				(1 << 5)

// Returns the scheduled events
uint32_t getScheduledEvents();
// Adds an event to be scheduled
void addSchedulerEvent(uint32_t input);
// Removes event to be scheduled
void removeSchedulerEvent();



#endif /* SCHEDULER_H_ */
