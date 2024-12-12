/*
 * Timer_Driver.c
 *
 *  Created on: Nov 22, 2024
 *      Author: codyl
 *      Tetris - Timer Driver Source Code
 *      Date: 12/11/2024
 */

#include <Timer_Driver.h>

TIM_HandleTypeDef TIM3_Config;
TIM_HandleTypeDef TIM5_Config;

// Timer to drop blocks
void timer3Init(){
	__HAL_RCC_TIM3_CLK_ENABLE();

	TIM3_Config.Instance = TIM3;
	TIM3_Config.Init.Prescaler =  1400; // 4150 for 3sec drop or 1400 for 1sec drop
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
}

void TIM3_Start(){
	  HAL_TIM_Base_Start_IT(&TIM3_Config);
}



/*
 * Tout = (Arr + 1)(Psc + 1)/Pclk
 * Tout = 1s
 * Pclk = 12.5 MHz
 * Arr = 1000
 * Psc = ?
 *
 * 1 = (1001)(Psc + 1)/12.5 * 10^6
 * 12.5*10^6 = 1001(Psc + 1)
 * 12488 = Psc + 1
 * 12487 = Psc
 *
 *315s in 48s
 *
 * 0.15238 = (1001)(12488)/Pclk
 */

// Timer set to 1s to keep track of game time
void timer5Init(){
	__HAL_RCC_TIM5_CLK_ENABLE();

	TIM5_Config.Instance = TIM5;
	TIM5_Config.Init.Prescaler = 12488;
	TIM5_Config.Init.CounterMode = TIM_COUNTERMODE_UP;
	TIM5_Config.Init.Period = (6563-1);
	TIM5_Config.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	TIM5_Config.Init.RepetitionCounter = 0;

	if (HAL_TIM_Base_Init(&TIM5_Config) != HAL_OK) {
	        // Initialization Error
	        while(1);
	    }

	HAL_NVIC_SetPriority(TIM5_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM5_IRQn);
}

void TIM5_Start(){
	  HAL_TIM_Base_Start_IT(&TIM5_Config);
}
