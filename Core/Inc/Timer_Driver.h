/*
 * Timer_Driver.h
 *
 *  Created on: Nov 22, 2024
 *      Author: codyl
 */

#ifndef INC_TIMER_DRIVER_H_
#define INC_TIMER_DRIVER_H_

#include "stm32f4xx_hal.h"

extern TIM_HandleTypeDef TIM3_Config;

void timer3Init();
void TIM3_Start();


#endif /* INC_TIMER_DRIVER_H_ */
