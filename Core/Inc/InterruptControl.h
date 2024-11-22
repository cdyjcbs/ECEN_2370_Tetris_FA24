/*
 * InterruptControl.h
 *
 *  Created on: November 11 2024
 *      Author: Cody Jacobs
 *      Lab 5 - Interrupt Control Header
 */


#include <STM32F4xx_hal.h>
#include <stdint.h>

#define NVIC_ISER0 ((volatile uint32_t*) 0xE000E100)	// Set-enable
#define NVIC_ICER0 ((volatile uint32_t*) 0xE000E180)	// Clear-enable
#define NVIC_ISPR0 ((volatile uint32_t*) 0xE000E200)	// Set-pending
#define NVIC_ICPR0 ((volatile uint32_t*) 0xE000E280)	// Clear-pending

#define EXTI0_IRQ0 6

// Enable IRQ
void enableIRQ(uint8_t IRQ_Num);
// Disable IRQ
void disableIRQ(uint8_t IRQ_Num);
// Clear pending IRQ
void clearPendingIRQ(uint8_t IRQ_Num);
// Set pending IRQ
void setPendingIRQ(uint8_t IRQ_Num);
// Clear EXTI Pending bit
void clearEXTIPendingIRQ(uint8_t Pin_Num);

void EXTI0_Enable();

