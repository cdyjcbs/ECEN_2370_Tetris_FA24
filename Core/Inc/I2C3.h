/*
 * I2C3.h
 *
 *  Created on: Nov 29, 2024
 *      Author: codyl
 */

#ifndef INC_I2C3_H_
#define INC_I2C3_H_

#include "stm32f4xx_hal.h"

extern I2C_HandleTypeDef I2C3_Config;

void I2C3_GPIO_Init();
void I2C3__Init();


#endif /* INC_I2C3_H_ */
