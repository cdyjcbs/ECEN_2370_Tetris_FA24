/*
 * I2C3.c
 *
 *  Created on: Nov 29, 2024
 *      Author: codyl
 */

#include <I2C3.h>

I2C_HandleTypeDef I2C3_Config;

// Button initialization prototype
void I2C3_GPIO_Init(){
	__HAL_RCC_I2C3_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	GPIO_InitTypeDef pinConfig = {0};
	pinConfig.Pin = GPIO_PIN_9; //GPIO_PIN_0
	pinConfig.Mode = GPIO_MODE_AF_OD;
	pinConfig.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	pinConfig.Pull = GPIO_PULLUP;
	pinConfig.Alternate = GPIO_AF4_I2C3;
	HAL_GPIO_Init(GPIOC, &pinConfig);

	pinConfig.Pin = GPIO_PIN_8;
	HAL_GPIO_Init(GPIOC, &pinConfig);
}

void I2C3__Init(){
	I2C3_Config.Instance = I2C3;
	I2C3_Config.Init.ClockSpeed = 100000;
	I2C3_Config.Init.DutyCycle = I2C_DUTYCYCLE_2;
	I2C3_Config.Init.OwnAddress1 = 0;
	I2C3_Config.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	I2C3_Config.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	I2C3_Config.Init.OwnAddress2 = 0;
	I2C3_Config.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	I2C3_Config.Init.NoStretchMode = I2C_GENERALCALL_DISABLE;
}



