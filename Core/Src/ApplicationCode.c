/*
 * ApplicationCode.c
 *
 *  Created on: Dec 30, 2023 (updated 11/12/2024) Thanks Donavon! 
 *      Author: Xavion
 *      Modified by: Cody Jacobs
 *      Tetris - Application Code Source
 *      Date: 12/11/2024
 */

#include <ApplicationCode.h>

/* Static variables */
extern void initialise_monitor_handles(void); 
extern TIM_HandleTypeDef TIM3_Config;

#if COMPILE_TOUCH_FUNCTIONS == 1
static STMPE811_TouchData StaticTouchData;
#if TOUCH_INTERRUPT_ENABLED == 1
static EXTI_HandleTypeDef LCDTouchIRQ;
void LCDTouchScreenInterruptGPIOInit(void);
#endif // TOUCH_INTERRUPT_ENABLED
#endif // COMPILE_TOUCH_FUNCTIONS

bool firstTouch = false;

void ApplicationInit(void)
{
	initialise_monitor_handles(); // Allows printf functionality
    LTCD__Init();
    LTCD_Layer_Init(0);
    LCD_Clear(0,LCD_COLOR_WHITE);
    // Draw welcome screen and await input to start game
    GameInit();

    #if COMPILE_TOUCH_FUNCTIONS == 1
	InitializeLCDTouch();

	// This is the orientation for the board to be direclty up where the buttons are vertically above the screen
	// Top left would be low x value, high y value. Bottom right would be low x value, low y value.
	StaticTouchData.orientation = STMPE811_Orientation_Portrait_2;

	#if TOUCH_INTERRUPT_ENABLED == 1
	LCDTouchScreenInterruptGPIOInit();
	#endif // TOUCH_INTERRUPT_ENABLED

	#endif // COMPILE_TOUCH_FUNCTIONS
}

void LCD_Visual_Demo(void)
{
	visualDemo();
}

// Initialize Button for IRQ, RNG, and Timers 3 and 5, then start both timers.
void Periph_Init(){
	buttonIRQInit();
	RNG_Init();
	timer3Init();
	timer5Init();
	TIM3_Start();
	TIM5_Start();
}

// IRQ Handler for User Button to Rotate Blocks
void EXTI0_IRQHandler(){
	HAL_NVIC_DisableIRQ(EXTI0_IRQn);
	addSchedulerEvent(ROTATE_BLOCK);
	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

// IRQ Handler for Timer 3 to drop blocks or end game
void TIM3_IRQHandler() {
	 if (__HAL_TIM_GET_FLAG(&TIM3_Config, TIM_FLAG_UPDATE)) {
	        // Clear the update interrupt flag
	        __HAL_TIM_CLEAR_FLAG(&TIM3_Config, TIM_FLAG_UPDATE);

	        // Check if area below block is open or full with another block
	        int full = isFull();

	        // If it is not full, then schedule block to drop
	        if (full != 0){
	        	addSchedulerEvent(DROP_BLOCK);
	        }

	        // Update the top row and if top row is at top of screen Game Over
			if (full == 0){
				int top_row = updateTop();
				if (top_row <= 1){
					gameOver();
				}
				// If top row is not at top of screen, schedule new block to fall
				else if (top_row > 1) {
					checkForTetris(top_row);
					addSchedulerEvent(NEW_BLOCK);
				}
			}
	 }
}

// IRQ Handler for Timer 5 to act as game timer
void TIM5_IRQHandler(){
	 if (__HAL_TIM_GET_FLAG(&TIM5_Config, TIM_FLAG_UPDATE)) {
		 // Clear the update interrupt flag
		 __HAL_TIM_CLEAR_FLAG(&TIM5_Config, TIM_FLAG_UPDATE);
		 addSecond();
	 }
}

#if COMPILE_TOUCH_FUNCTIONS == 1
void LCD_Touch_Polling_Demo(void)
{
	LCD_Clear(0,LCD_COLOR_GREEN);
	while (1) {
		/* If touch pressed */
		if (returnTouchStateAndLocation(&StaticTouchData) == STMPE811_State_Pressed) {
			/* Touch valid */
			if (StaticTouchData.x < 120) {
				LCD_Clear(0, LCD_COLOR_RED);
//				eraseCurrentBlock();
//				updateXPos(1);
//				drawCurrentBlock();
			}
			if (StaticTouchData.x >= 120){
				LCD_Clear(0, LCD_COLOR_GREEN);
//				eraseCurrentBlock();
//				updateXPos(2);
//				drawCurrentBlock();
			}
//
//			printf("\nX: %03d\nY: %03d\n", StaticTouchData.x, StaticTouchData.y);
//			LCD_Clear(0, LCD_COLOR_RED);
		} else {
			/* Touch not pressed */
			printf("Not Pressed\n\n");
			LCD_Clear(0, LCD_COLOR_GREEN);
		}
	}
}

void LCD_Touch_Polling(void)
{
//	int h = 0;
	while (firstTouch == false) {
		/* If touch pressed */
		if (returnTouchStateAndLocation(&StaticTouchData) == STMPE811_State_Pressed) {
						/* Touch valid */

			addSchedulerEvent(GAME_START);
				firstTouch = true;
		}
//						if (StaticTouchData.x < 120) {
//							int canMove = canMoveLeft();
//							if (canMove == 1){
//								eraseCurrentBlock();
//								updateXpos(1);
//								drawCurrentBlock();
//							}
//						}
//						if (StaticTouchData.x >= 120){
//							eraseCurrentBlock();
//							updateXpos(2);
//							drawCurrentBlock();
//						}
//			printf("\nX: %03d\nY: %03d\n", StaticTouchData.x, StaticTouchData.y);
//			moveBlock(StaticTouchData.x, StaticTouchData.y, h);
	}
}


// TouchScreen Interrupt
#if TOUCH_INTERRUPT_ENABLED == 1

void LCDTouchScreenInterruptGPIOInit(void)
{
	GPIO_InitTypeDef LCDConfig = {0};
    LCDConfig.Pin = GPIO_PIN_15;
    LCDConfig.Mode = GPIO_MODE_IT_RISING_FALLING;
    LCDConfig.Pull = GPIO_NOPULL;
    LCDConfig.Speed = GPIO_SPEED_FREQ_HIGH;
    
    // Clock enable
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // GPIO Init      
    HAL_GPIO_Init(GPIOA, &LCDConfig);

    // Interrupt Configuration
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

	LCDTouchIRQ.Line = EXTI_LINE_15;

}

#define TOUCH_DETECTED_IRQ_STATUS_BIT   (1 << 0)  // Touchscreen detected bitmask

static uint8_t statusFlag;

// IRQ Handler for the Touch Screen
void EXTI15_10_IRQHandler()
{
	HAL_NVIC_DisableIRQ(EXTI15_10_IRQn); // May consider making this a universial interrupt guard
	bool isTouchDetected = false;

	uint8_t ctrlReg = ReadRegisterFromTouchModule(STMPE811_TSC_CTRL);
		if (ctrlReg & 0x80)
		{
			isTouchDetected = true;
		}

	static uint32_t count;
	count = 0;
	while(count == 0){
		count = STMPE811_Read(STMPE811_FIFO_SIZE);
	}

	// Determine if it is pressed or unpressed
	if(isTouchDetected) // Touch has been detected
	{
		// If it is the first touch, then this will trigger Game to Start
		if (!firstTouch){
			addSchedulerEvent(GAME_START);
			firstTouch = true;
		}

		// Subsequent touches will schedule the block to move left or right depending on touch position
		else {
			printf("\nPressed");
			DetermineTouchPosition(&StaticTouchData);

			/* Touch valid */
			if (StaticTouchData.x < 120 && canMoveLeft()) {
				addSchedulerEvent(MOVE_LEFT);
			}
			else if (StaticTouchData.x >= 120 && canMoveRight()){
				addSchedulerEvent(MOVE_RIGHT);
			}
		}
	}
	else{
		/* Touch not pressed */
		printf("\nNot pressed \n");
	}

// Disable touch interrupt bit on the STMPE811
	uint8_t currentIRQEnables = ReadRegisterFromTouchModule(STMPE811_INT_EN);
	WriteDataToTouchModule(STMPE811_INT_EN, 0x00);

// Clear the interrupt bit in the STMPE811
	statusFlag = ReadRegisterFromTouchModule(STMPE811_INT_STA);
	uint8_t clearIRQData = (statusFlag | TOUCH_DETECTED_IRQ_STATUS_BIT); // Write one to clear bit
	WriteDataToTouchModule(STMPE811_INT_STA, clearIRQData);

	STMPE811_Write(STMPE811_FIFO_STA, 0x01);
	STMPE811_Write(STMPE811_FIFO_STA, 0x00);

	// Re-enable IRQs
    WriteDataToTouchModule(STMPE811_INT_EN, currentIRQEnables);
	HAL_EXTI_ClearPending(&LCDTouchIRQ, EXTI_TRIGGER_RISING_FALLING);

	HAL_NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}
#endif // TOUCH_INTERRUPT_ENABLED
#endif // COMPILE_TOUCH_FUNCTIONS
