/*
 * InterruptControl.c
 *
 *  Created on: November 12, 2024
 *      Author: Cody Jacobs
 *      Lab 5 - Interrupt Control Source
 */



#include <InterruptControl.h>


// Enable IRQ - Set Enable
void enableIRQ(uint8_t IRQ_Num){
	if (IRQ_Num < 32){
		(*NVIC_ISER0 |= (0x1 << IRQ_Num));
	}
	else if (IRQ_Num > 32){
		uint32_t bitFields = (IRQ_Num % 32);
		(*NVIC_ISER0 |= (0x1 << bitFields));
	}
}

// Disable IRQ - Clear Enable
void disableIRQ(uint8_t IRQ_Num){
	if (IRQ_Num < 32){
			(*NVIC_ICER0 |= (0x1 << IRQ_Num));
		}
		else if (IRQ_Num > 32){
			uint32_t bitFields = (IRQ_Num % 32);
			(*NVIC_ICER0 |= (0x1 << bitFields));
		}
}

// Clear pending IRQ - Clear Pending
void clearPendingIRQ(uint8_t IRQ_Num){
	if (IRQ_Num < 32){
			(*NVIC_ICPR0 |= (0x1 << IRQ_Num));
		}
		else if (IRQ_Num > 32){
			uint32_t bitFields = (IRQ_Num % 32);
			(*NVIC_ICPR0 |= (0x1 << bitFields));
		};
}

// Set pending IRQ - Set Pending
void setPendingIRQ(uint8_t IRQ_Num){
	if (IRQ_Num < 32){
			(*NVIC_ISPR0 |= (0x1 << IRQ_Num));
		}
		else if (IRQ_Num > 32){
			uint32_t bitFields = (IRQ_Num % 32);
			(*NVIC_ISPR0 |= (0x1 << bitFields));
		};
}

// Clear EXTI Pending bit
void clearEXTIPendingIRQ(uint8_t Pin_Num){
	EXTI->PR |= (0x1 << Pin_Num);
}

void EXTI0_Enable(){
	EXTI_HandleTypeDef handle_config;
	EXTI_ConfigTypeDef exti_config;
	exti_config.Line = EXTI_LINE_0;
	exti_config.Mode = EXTI_MODE_INTERRUPT;
	exti_config.Trigger = EXTI_TRIGGER_FALLING;
	exti_config.GPIOSel = EXTI_GPIOA;
	handle_config.Line = EXTI_LINE_0;
	HAL_EXTI_SetConfigLine(&handle_config, &exti_config);
}
