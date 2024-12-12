/*
 * Button_Driver.c
 *
 *  Created on: November 12, 2024
 *      Author: Cody Jacobs
 *      Tetris - Button Driver Source
 *		Date: 12/11/2024
 */

#include <Button_Driver.h>

// Button initialization prototype
void buttonInit(){
	GPIO_InitTypeDef pinConfig;
	pinConfig.Pin = USER_BUTTON_PIN; //GPIO_PIN_0
	pinConfig.Mode = GPIO_MODE_INPUT;
	pinConfig.Speed = GPIO_SPEED_FREQ_HIGH;
	pinConfig.Pull = GPIO_NOPULL;
//	pinConfig.Alternate = GPIO_AF0_RTC_50Hz;
	enableClock();
	HAL_GPIO_Init(GPIOA, &pinConfig);
}

// Enable clock Prototype
void enableClock(){
	__HAL_RCC_GPIOA_CLK_ENABLE();
}

// Button pressed Boolean
bool buttonPressed(){
	bool returnValue = false;
	int pinValue = HAL_GPIO_ReadPin(USER_BUTTON_PORT, USER_BUTTON_PIN);
	if (pinValue == USER_BUTTON_PRESSED){
		returnValue = true;
	}
	return returnValue;
}

// Button initialization for Interrupt Mode
void buttonIRQInit(){
	GPIO_InitTypeDef pinConfig = {0};
	pinConfig.Pin = GPIO_PIN_0; //GPIO_PIN_0
	pinConfig.Mode = GPIO_MODE_IT_FALLING;
	pinConfig.Speed = GPIO_SPEED_FREQ_HIGH;
	pinConfig.Pull = GPIO_NOPULL;
	__HAL_RCC_GPIOA_CLK_ENABLE();
	HAL_GPIO_Init(GPIOA, &pinConfig);

	HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

