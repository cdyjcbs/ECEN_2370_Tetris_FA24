/*
 * RNG.c
 *
 *  Created on: Nov 22, 2024
 *      Author: codyl
 */

#include <RNG.h>

RNG_HandleTypeDef RNG_Config;

void RNG_Init(){
	__HAL_RCC_RNG_CLK_ENABLE();
	RNG_Config.Instance = RNG;
	if (HAL_RNG_Init(&RNG_Config) != HAL_OK){
		for (;;);
	}
}

uint32_t GetRandomBlock(){
	uint32_t random_num = 0;
	random_num = HAL_RNG_GetRandomNumber(&RNG_Config);
	random_num = (random_num % 7) + 1;
	return random_num;
}

uint32_t GetRandomOrientation(){
	uint32_t random_num = 0;
	random_num = HAL_RNG_GetRandomNumber(&RNG_Config);
	random_num = (random_num % 4) + 1;
	return random_num;
}
