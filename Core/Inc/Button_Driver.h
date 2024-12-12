/*
 * Button_Driver.h
 *
 *  Created on: November 12, 2024
 *      Author: Cody Jacobs
 *      Tetris - Button Driver Header
 *      Date: 12/11/2024
 */

#ifndef BUTTON_DRIVER_H_
#define BUTTON_DRIVER_H_

#include <stdbool.h>
#include <InterruptControl.h>

#define USER_BUTTON_PORT GPIOA
#define USER_BUTTON_PIN	0
#define USER_BUTTON_PRESSED 1
#define USER_BUTTON_NOT_PRESSED 0

// Button initialization prototype
void buttonInit();
// Enable clock Prototype
void enableClock();
// Button pressed Boolean
bool buttonPressed();
// Button initialization for Interrupt Mode
void buttonIRQInit();
#endif /* BUTTON_DRIVER_H_ */
