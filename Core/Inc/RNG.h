/*
 * RNG.h
 *
 *  Created on: Nov 22, 2024
 *      Author: codyl
 */

#include "stm32f4xx_hal.h"

void RNG_Init();
uint32_t GetRandomBlock();
uint32_t GetRandomOrientation();
