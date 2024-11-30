/*
 * Timer_Driver.c
 *
 *  Created on: Nov 22, 2024
 *      Author: codyl
 */

#include <Timer_Driver.h>

TIM_HandleTypeDef TIM3_Config;


void timer3Init(){
	__HAL_RCC_TIM3_CLK_ENABLE();

	TIM3_Config.Instance = TIM3;
	TIM3_Config.Init.Prescaler = 1000;
	TIM3_Config.Init.CounterMode = TIM_COUNTERMODE_UP;
	TIM3_Config.Init.Period = (65536-1);
	TIM3_Config.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	TIM3_Config.Init.RepetitionCounter = 0;

	if (HAL_TIM_Base_Init(&TIM3_Config) != HAL_OK) {
	        // Initialization Error
	        while(1);
	    }

	HAL_NVIC_SetPriority(TIM3_IRQn, 3, 0);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);

//	HAL_TIM_Base_Start_IT(&TIM3_Config);
}

void TIM3_Start(){
	  HAL_TIM_Base_Start_IT(&TIM3_Config);
}
