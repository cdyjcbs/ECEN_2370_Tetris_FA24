/*
 * Button_Driver.c
 *
 *  Created on: November 12, 2024
 *      Author: Cody Jacobs
 *      Lab 5 - Button Driver Source
 */

#include <Button_Driver.h>

// Button initialization prototype
void buttonInit(){
//	typedef struct {
//		uint8_t PinNumber;		// Pin Number
//		uint8_t PinMode;		// Pin Mode
//		uint8_t OPType;			// Output Type
//		uint8_t PinSpeed;		// Pin Speed
//		uint8_t PinPuPdControl;	// Pin Push up/ Pull Down Control
//		uint8_t PinAltFunMode;	// Alternate Function Mode
//	} Button;

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
//	GPIO_ClockControl(USER_BUTTON_PORT, 0);
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

//	HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

