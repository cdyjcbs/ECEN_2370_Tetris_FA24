/*
 * Timer_Driver.h
 *
 *  Created on: Nov 22, 2024
 *      Author: codyl
 *		Tetris - Timer Driver Header
 *		Date: 12/11/2024
 */

#ifndef INC_TIMER_DRIVER_H_
#define INC_TIMER_DRIVER_H_

#include "stm32f4xx_hal.h"

extern TIM_HandleTypeDef TIM3_Config;
extern TIM_HandleTypeDef TIM5_Config;


void timer3Init();
void TIM3_Start();

void timer5Init();
void TIM5_Start();


#endif /* INC_TIMER_DRIVER_H_ */
