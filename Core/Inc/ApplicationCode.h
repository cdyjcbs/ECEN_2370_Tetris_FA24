/*
 * ApplicationCode.h
 *
 *  Created on: Dec 30, 2023
 *      Author: Xavion
 *      Modified by: Cody Jacobs
 *      Tetris - Application Code Header
 *      Date: 12/11/2024
 */

#include <LCD_Driver.h>
#include <Button_Driver.h>
#include <Scheduler.h>
#include <stdio.h>

#ifndef INC_APPLICATIONCODE_H_
#define INC_APPLICATIONCODE_H_

void LCDTouchScreenInterruptGPIOInit(void);
void ApplicationInit(void);
void LCD_Visual_Demo(void);
void Periph_Init();

#if (COMPILE_TOUCH_FUNCTIONS == 1) && (COMPILE_TOUCH_INTERRUPT_SUPPORT == 0)
void LCD_Touch_Polling_Demo(void);
#endif // (COMPILE_TOUCH_FUNCTIONS == 1) && (COMPILE_TOUCH_INTERRUPT_SUPPORT == 0)

#endif /* INC_APPLICATIONCODE_H_ */
