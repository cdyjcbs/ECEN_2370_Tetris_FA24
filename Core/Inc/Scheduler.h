/*
 * Scheduler.h
 *
 *  Created on: November 12, 2024
 *      Author: Cody Jacobs
 *      Tetris - Scheduler Header
 *      Date: 12/11/2024
 */

#include <stdint.h>

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#define LED_TOGGLE_EVENT			(1 << 0)
#define LED_DELAY_EVENT				(1 << 1)
#define LED_POLL_EVENT				(1 << 2)
#define APP_DELAY_FLAG_EVENT 		(1 << 3)
#define NEW_BLOCK				 	(1 << 4)
#define ROTATE_BLOCK				(1 << 5)
#define DROP_BLOCK					(1 << 6)
#define MOVE_LEFT					(1 << 7)
#define MOVE_RIGHT					(1 << 8)
#define GAME_OVER					(1 << 9)
#define GAME_START					(1 << 10)

// Returns the scheduled events
uint32_t getScheduledEvents();
// Adds an event to be scheduled
void addSchedulerEvent(uint32_t input);
// Removes event to be scheduled
void removeSchedulerEvent();



#endif /* SCHEDULER_H_ */
