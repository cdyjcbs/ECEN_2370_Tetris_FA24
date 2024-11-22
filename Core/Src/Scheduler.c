/*
 * Scheduler.c
 *
 *  Created on: November 12, 2024
 *      Author: Cody Jacobs
 *      Lab 5 - Scheduler Source
 */

#include <Scheduler.h>

static uint32_t scheduledEvents;

// Returns the scheduled events
uint32_t getScheduledEvents(){
	return scheduledEvents;
}

// Adds an event to be scheduled
void addSchedulerEvent(uint32_t event){
	scheduledEvents |= event;
}

// Removes event to be scheduled
void removeSchedulerEvent(uint32_t event){
	scheduledEvents &= ~(event);
}
