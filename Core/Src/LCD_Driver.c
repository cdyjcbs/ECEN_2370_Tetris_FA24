/*
 * LCD_Driver.c
 *
 *  Created on: Sep 28, 2023
 *      Author: Xavion
 */

#include <LCD_Driver.h>

/**
  * @brief LTDC Initialization Function
  * @param None
  * @retval None
  */

static LTDC_HandleTypeDef hltdc;
static RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;
static FONT_t *LCD_Currentfonts;
static uint16_t CurrentTextColor   = 0xFFFF;

static uint16_t GameScreenColor = LCD_COLOR_GREY;

uint16_t Obj_Color[16][12] = {0};
int movingBlock[16][12] = {0};
int LCD_Screen[16][12] = {0};
int blockSize = 20;

static uint16_t CurrentBlock[4] = {0};

/*
 * fb[y*W+x] OR fb[y][x]
 * Alternatively, we can modify the linker script to have an end address of 20013DFB instead of 2002FFFF, so it does not place variables in the same region as the frame buffer. In this case it is safe to just specify the raw address as frame buffer.
 */
//uint32_t frameBuffer[(LCD_PIXEL_WIDTH*LCD_PIXEL_WIDTH)/2] = {0};		//16bpp pixel format. We can size to uint32. this ensures 32 bit alignment


//Someone from STM said it was "often accessed" a 1-dim array, and not a 2d array. However you still access it like a 2dim array,  using fb[y*W+x] instead of fb[y][x].
uint16_t frameBuffer[LCD_PIXEL_WIDTH*LCD_PIXEL_HEIGHT] = {0};			//16bpp pixel format.


void LCD_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable the LTDC clock */
  __HAL_RCC_LTDC_CLK_ENABLE();

  /* Enable GPIO clock */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /* GPIO Config
   *
    LCD pins
   LCD_TFT R2 <-> PC.10
   LCD_TFT G2 <-> PA.06
   LCD_TFT B2 <-> PD.06
   LCD_TFT R3 <-> PB.00
   LCD_TFT G3 <-> PG.10
   LCD_TFT B3 <-> PG.11
   LCD_TFT R4 <-> PA.11
   LCD_TFT G4 <-> PB.10
   LCD_TFT B4 <-> PG.12
   LCD_TFT R5 <-> PA.12
   LCD_TFT G5 <-> PB.11
   LCD_TFT B5 <-> PA.03
   LCD_TFT R6 <-> PB.01
   LCD_TFT G6 <-> PC.07
   LCD_TFT B6 <-> PB.08
   LCD_TFT R7 <-> PG.06
   LCD_TFT G7 <-> PD.03
   LCD_TFT B7 <-> PB.09
   LCD_TFT HSYNC <-> PC.06
   LCDTFT VSYNC <->  PA.04
   LCD_TFT CLK   <-> PG.07
   LCD_TFT DE   <->  PF.10
  */

  /* GPIOA configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_6 |
                           GPIO_PIN_11 | GPIO_PIN_12;
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
  GPIO_InitStructure.Alternate= GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

 /* GPIOB configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_8 | \
                           GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

 /* GPIOC configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_10;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

 /* GPIOD configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_3 | GPIO_PIN_6;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

 /* GPIOF configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_10;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStructure);

 /* GPIOG configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_6 | GPIO_PIN_7 | \
                           GPIO_PIN_11;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);

  /* GPIOB configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1;
  GPIO_InitStructure.Alternate= GPIO_AF9_LTDC;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* GPIOG configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_10 | GPIO_PIN_12;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);
}

void LTCD_Layer_Init(uint8_t LayerIndex)
{
	LTDC_LayerCfgTypeDef  pLayerCfg;

	pLayerCfg.WindowX0 = 0;	//Configures the Window HORZ START Position.
	pLayerCfg.WindowX1 = LCD_PIXEL_WIDTH;	//Configures the Window HORZ Stop Position.
	pLayerCfg.WindowY0 = 0;	//Configures the Window vertical START Position.
	pLayerCfg.WindowY1 = LCD_PIXEL_HEIGHT;	//Configures the Window vertical Stop Position.
	pLayerCfg.PixelFormat = LCD_PIXEL_FORMAT_1;  //INCORRECT PIXEL FORMAT WILL GIVE WEIRD RESULTS!! IT MAY STILL WORK FOR 1/2 THE DISPLAY!!! //This is our buffers pixel format. 2 bytes for each pixel
	pLayerCfg.Alpha = 255;
	pLayerCfg.Alpha0 = 0;
	pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
	pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
	if (LayerIndex == 0){
		pLayerCfg.FBStartAdress = (uintptr_t)frameBuffer;
	}
	pLayerCfg.ImageWidth = LCD_PIXEL_WIDTH;
	pLayerCfg.ImageHeight = LCD_PIXEL_HEIGHT;
	pLayerCfg.Backcolor.Blue = 0;
	pLayerCfg.Backcolor.Green = 0;
	pLayerCfg.Backcolor.Red = 0;
	if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, LayerIndex) != HAL_OK)
	{
		LCD_Error_Handler();
	}
}

void clearScreen(void)
{
  LCD_Clear(0,LCD_COLOR_WHITE);
}

void LTCD__Init(void)
{
	hltdc.Instance = LTDC;
	/* Configure horizontal synchronization width */
	hltdc.Init.HorizontalSync = ILI9341_HSYNC;
	/* Configure vertical synchronization height */
	hltdc.Init.VerticalSync = ILI9341_VSYNC;
	/* Configure accumulated horizontal back porch */
	hltdc.Init.AccumulatedHBP = ILI9341_HBP;
	/* Configure accumulated vertical back porch */
	hltdc.Init.AccumulatedVBP = ILI9341_VBP;
	/* Configure accumulated active width */
	hltdc.Init.AccumulatedActiveW = 269;
	/* Configure accumulated active height */
	hltdc.Init.AccumulatedActiveH = 323;
	/* Configure total width */
	hltdc.Init.TotalWidth = 279;
	/* Configure total height */
	hltdc.Init.TotalHeigh = 327;
	/* Configure R,G,B component values for LCD background color */
	hltdc.Init.Backcolor.Red = 0;
	hltdc.Init.Backcolor.Blue = 0;
	hltdc.Init.Backcolor.Green = 0;

	/* LCD clock configuration */
	/* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
	/* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAIN = 192 Mhz */
	/* PLLLCDCLK = PLLSAI_VCO Output/PLLSAIR = 192/4 = 48 Mhz */
	/* LTDC clock frequency = PLLLCDCLK / LTDC_PLLSAI_DIVR_8 = 48/4 = 6Mhz */

	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
	PeriphClkInitStruct.PLLSAI.PLLSAIN = 192;
	PeriphClkInitStruct.PLLSAI.PLLSAIR = 4;
	PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_8;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
	/* Polarity */
	hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
	hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
	hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
	hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;

	LCD_GPIO_Init();

	if (HAL_LTDC_Init(&hltdc) != HAL_OK)
	 {
	   LCD_Error_Handler();
	 }

	ili9341_Init();
}

/* START Draw functions */


/*
 * This is really the only function needed.
 * All drawing consists of is manipulating the array.
 * Adding input sanitation should probably be done.
 */
void LCD_Draw_Pixel(uint16_t x, uint16_t y, uint16_t color)
{
	frameBuffer[y*LCD_PIXEL_WIDTH+x] = color;  //You cannot do x*y to set the pixel.
}

/*
 * These functions are simple examples. Most computer graphics like OpenGl and stm's graphics library use a state machine. Where you first call some function like SetColor(color), SetPosition(x,y), then DrawSqure(size)
 * Instead all of these are explicit where color, size, and position are passed in.
 * There is tons of ways to handle drawing. I dont think it matters too much.
 */
void LCD_Draw_Circle_Fill(uint16_t Xpos, uint16_t Ypos, uint16_t radius, uint16_t color)
{
    for(int16_t y=-radius; y<=radius; y++)
    {
        for(int16_t x=-radius; x<=radius; x++)
        {
            if(x*x+y*y <= radius*radius)
            {
            	LCD_Draw_Pixel(x+Xpos, y+Ypos, color);
            }
        }
    }
}

void LCD_Draw_Square_Fill(uint16_t Xpos, uint16_t Ypos, uint16_t size, uint16_t color)
{
    for(int16_t i=1; i<=size; i++)
    {
        for(int16_t j=1; j<=size; j++)
        {
            LCD_Draw_Pixel(i+Xpos, j+Ypos, color);
        }
    }
}

void LCD_Draw_Square_Fill_Border(uint16_t Xpos, uint16_t Ypos, uint16_t color)
{
	uint16_t x = Xpos*blockSize;
	uint16_t y = Ypos*blockSize;
	for(int16_t i=1; i<=blockSize; i++)
	    {
	        for(int16_t j=1; j<=blockSize; j++)
	        {
	            LCD_Draw_Pixel(i+x, j+y, color);
	            LCD_Draw_Vertical_Line(x,y,blockSize,LCD_COLOR_BLACK);
	            LCD_Draw_Vertical_Line(x+blockSize,y,blockSize+1,LCD_COLOR_BLACK);
	            LCD_Draw_Horizontal_Line(x,y,blockSize,LCD_COLOR_BLACK);
	            LCD_Draw_Horizontal_Line(x,y+blockSize,blockSize,LCD_COLOR_BLACK);
	        }
	    }
}

void LCD_Erase_Square(uint16_t Xpos, uint16_t Ypos)
{
	uint16_t x = Xpos*blockSize;
	uint16_t y = Ypos*blockSize;
	for(int16_t i=1; i<=blockSize; i++)
	    {
	        for(int16_t j=1; j<=blockSize; j++)
	        {
	            LCD_Draw_Pixel(i+x, j+y, GameScreenColor);
	            LCD_Draw_Vertical_Line(x,y,blockSize,GameScreenColor);
	            LCD_Draw_Vertical_Line(x+blockSize,y,blockSize+1,GameScreenColor);
	            LCD_Draw_Horizontal_Line(x,y,blockSize,GameScreenColor);
	            LCD_Draw_Horizontal_Line(x,y+blockSize,blockSize,GameScreenColor);
	        }
	    }
}

void LCD_Draw_Vertical_Line(uint16_t x, uint16_t y, uint16_t len, uint16_t color)
{
  for (uint16_t i = 0; i < len; i++)
  {
	  LCD_Draw_Pixel(x, i+y, color);
  }
}

void LCD_Draw_Horizontal_Line(uint16_t x, uint16_t y, uint16_t len, uint16_t color)
{
  for (uint16_t i = 0; i < len; i++)
  {
	  LCD_Draw_Pixel(i+x, y, color);
  }
}

void LCD_Clear(uint8_t LayerIndex, uint16_t Color)
{
	if (LayerIndex == 0){
		for (uint32_t i = 0; i < LCD_PIXEL_WIDTH * LCD_PIXEL_HEIGHT; i++){
			frameBuffer[i] = Color;
		}
	}
  // TODO: Add more Layers if needed
}

//This was taken and adapted from stm32's mcu code
void LCD_SetTextColor(uint16_t Color)
{
  CurrentTextColor = Color;
}

//This was taken and adapted from stm32's mcu code
void LCD_SetFont(FONT_t *fonts)
{
  LCD_Currentfonts = fonts;
}

//This was taken and adapted from stm32's mcu code
void LCD_Draw_Char(uint16_t Xpos, uint16_t Ypos, const uint16_t *c)
{
  uint32_t index = 0, counter = 0;
  for(index = 0; index < LCD_Currentfonts->Height; index++)
  {
    for(counter = 0; counter < LCD_Currentfonts->Width; counter++)
    {
      if((((c[index] & ((0x80 << ((LCD_Currentfonts->Width / 12 ) * 8 ) ) >> counter)) == 0x00) && (LCD_Currentfonts->Width <= 12)) || (((c[index] & (0x1 << counter)) == 0x00)&&(LCD_Currentfonts->Width > 12 )))
      {
         //Background If want to overrite text under then add a set color here
      }
      else
      {
    	  LCD_Draw_Pixel(counter + Xpos,index + Ypos,CurrentTextColor);
      }
    }
  }
}

//This was taken and adapted from stm32's mcu code
void LCD_DisplayChar(uint16_t Xpos, uint16_t Ypos, uint8_t Ascii)
{
  Ascii -= 32;
  LCD_Draw_Char(Xpos, Ypos, &LCD_Currentfonts->table[Ascii * LCD_Currentfonts->Height]);
}

void visualDemo(void)
{
	uint16_t x;
	uint16_t y;
	// This for loop just illustrates how with using logic and for loops, you can create interesting things
	// this may or not be useful ;)
	for(y=0; y<LCD_PIXEL_HEIGHT; y++){
		for(x=0; x < LCD_PIXEL_WIDTH; x++){
			if (x & 32)
				frameBuffer[x*y] = LCD_COLOR_WHITE;
			else
				frameBuffer[x*y] = LCD_COLOR_BLACK;
		}
	}

	HAL_Delay(1500);
	LCD_Clear(0, LCD_COLOR_GREEN);
	HAL_Delay(1500);
	LCD_Clear(0, LCD_COLOR_RED);
	HAL_Delay(1500);
	LCD_Clear(0, LCD_COLOR_WHITE);
	LCD_Draw_Vertical_Line(10,10,250,LCD_COLOR_MAGENTA);
	HAL_Delay(1500);
	LCD_Draw_Horizontal_Line(10,10,100,LCD_COLOR_MAGENTA);
	HAL_Delay(1500);
	LCD_Draw_Vertical_Line(230,10,250,LCD_COLOR_MAGENTA);
	HAL_Delay(1500);
	LCD_Draw_Horizontal_Line(230,10,100,LCD_COLOR_MAGENTA);
	HAL_Delay(1500);

	LCD_Draw_Circle_Fill(125,150,20,LCD_COLOR_BLACK);
	HAL_Delay(2000);
	LCD_Draw_Square_Fill(25,50,20,LCD_COLOR_BLACK);
	HAL_Delay(2000);

	LCD_Clear(0,LCD_COLOR_WHITE);
	LCD_SetTextColor(LCD_COLOR_BLACK);
	LCD_SetFont(&Font16x24);

	LCD_DisplayChar(100,140,'H');
	LCD_DisplayChar(115,140,'e');
	LCD_DisplayChar(125,140,'l');
	LCD_DisplayChar(130,140,'l');
	LCD_DisplayChar(140,140,'o');

	LCD_DisplayChar(100,160,'W');
	LCD_DisplayChar(115,160,'o');
	LCD_DisplayChar(125,160,'r');
	LCD_DisplayChar(130,160,'l');
	LCD_DisplayChar(140,160,'d');
//	blockOne();
}

void GameInit(void)
{
	uint16_t x;
	uint16_t y;
	uint16_t size = 10;
	// This for loop just illustrates how with using logic and for loops, you can create interesting things
	// this may or not be useful ;)
//	for(y=0; y<LCD_PIXEL_HEIGHT; y++){
//		for(x=0; x < LCD_PIXEL_WIDTH; x++){
//			if (x & 32)
//				frameBuffer[x*y] = LCD_COLOR_WHITE;
//			else
//				frameBuffer[x*y] = LCD_COLOR_BLACK;
//		}
//	}

	LCD_Clear(0,GameScreenColor);
	LCD_SetTextColor(LCD_COLOR_WHITE);
	LCD_SetFont(&Font16x24);

	LCD_DisplayChar(80,140,'W');
	LCD_DisplayChar(94,140,'e');
	LCD_DisplayChar(104,140,'l');
	LCD_DisplayChar(112,140,'c');
	LCD_DisplayChar(124,140,'o');
	LCD_DisplayChar(140,140,'m');
	LCD_DisplayChar(155,140,'e');

	LCD_DisplayChar(110,165,'t');
	LCD_DisplayChar(120,165,'o');

	LCD_DisplayChar(95,190,'T');
	LCD_DisplayChar(105,190,'e');
	LCD_DisplayChar(115,190,'t');
	LCD_DisplayChar(125,190,'r');
	LCD_DisplayChar(132,190,'i');
	LCD_DisplayChar(139,190,'s');



//	blockOneH(45,25,10,LCD_COLOR_CYAN);
//	blockTwo(95,25,10,LCD_COLOR_BLUE);
//	blockThree(135,25,10,LCD_COLOR_ORANGE);
//	blockFour(175,25,10,LCD_COLOR_YELLOW);
//	blockFive(65,55,10,LCD_COLOR_GREEN);
//	blockSix(105,55,10,LCD_COLOR_MAGENTA);
//	blockSeven(155,55,10,LCD_COLOR_RED);
//
//	blockOneV(45,85,10,LCD_COLOR_CYAN);

	LCD_Clear(0,GameScreenColor);
	LCD_SetTextColor(LCD_COLOR_WHITE);
	LCD_SetFont(&Font16x24);

	drawBlockSeven(5, 2, 1);
	updateCurrentBlock(7, 5, 2, 1);
	HAL_Delay(1000);

//	eraseBlockSeven(5, 2, 1);
//	drawBlockSeven(5, 6, 2);
//	HAL_Delay(1000);
//
//	eraseBlockSeven(5, 6, 2);
//	drawBlockSeven(5, 10, 3);
//	HAL_Delay(1000);
//
//	eraseBlockSeven(5, 10, 3);
//	drawBlockSeven(5, 14, 4);

//	for (int i = 3; i < 15; i++){
//		drawBlockOne(5, i, 1);
//		HAL_Delay(1000);
//		eraseBlockOne(5, i, 1);
//
//		drawBlockOne(5, i, 2);
//		HAL_Delay(1000);
//		eraseBlockOne(5, i, 2);
//
//		drawBlockOne(5, i, 3);
//		HAL_Delay(1000);
//		eraseBlockOne(5, i, 3);
//
//		drawBlockOne(5, i, 4);
//		HAL_Delay(1000);
//		eraseBlockOne(5, i, 4);
//	}
//	drawBlockOne(5,15,1);
}

void drawBlockOne(uint16_t Xpos, uint16_t Ypos, uint16_t orientation){
	if (orientation == 1 || orientation == 3){
		for (int i = Xpos; i < (Xpos+4); i++){
			LCD_Draw_Square_Fill_Border(i,Ypos,LCD_COLOR_CYAN);
		}
	}
	if (orientation == 2 || orientation == 4){
		for (int i = Ypos-1; i < (Ypos+3); i++){
			LCD_Draw_Square_Fill_Border(Xpos+1,i,LCD_COLOR_CYAN);
		}
	}
}

void eraseBlockOne(uint16_t Xpos, uint16_t Ypos, uint16_t orientation){
	if (orientation == 1 || orientation == 3){
		for (int i = Xpos; i < (Xpos+4); i++){
			LCD_Erase_Square(i, Ypos);
		}
	}
	if (orientation == 2 || orientation == 4){
		for (int i = Ypos-1; i < (Ypos+3); i++){
			LCD_Erase_Square(Xpos+1, i);
		}
	}
}

void drawBlockTwo(uint16_t Xpos, uint16_t Ypos, uint16_t orientation){
	if (orientation == 1){
		for (int i = Xpos; i < (Xpos+3); i++){
			LCD_Draw_Square_Fill_Border(i,Ypos,LCD_COLOR_BLUE);
		}
		LCD_Draw_Square_Fill_Border(Xpos,Ypos-1,LCD_COLOR_BLUE);
	}
	if (orientation == 2){
			for (int i = Ypos; i < (Ypos+3); i++){
				LCD_Draw_Square_Fill_Border(Xpos+1,i,LCD_COLOR_BLUE);
			}
			LCD_Draw_Square_Fill_Border(Xpos+2,Ypos,LCD_COLOR_BLUE);
		}
	if (orientation == 3){
		for (int i = Xpos; i < (Xpos+3); i++){
			LCD_Draw_Square_Fill_Border(i,Ypos,LCD_COLOR_BLUE);
		}
		LCD_Draw_Square_Fill_Border(Xpos+2,Ypos+1,LCD_COLOR_BLUE);
	}
	if (orientation == 4){
		for (int i = Ypos-1; i < (Ypos+2); i++){
			LCD_Draw_Square_Fill_Border(Xpos+1,i,LCD_COLOR_BLUE);
		}
		LCD_Draw_Square_Fill_Border(Xpos,Ypos+1,LCD_COLOR_BLUE);
	}
}

void eraseBlockTwo(uint16_t Xpos, uint16_t Ypos, uint16_t orientation){
	if (orientation == 1){
		for (int i = Xpos; i < (Xpos+3); i++){
			LCD_Erase_Square(i,Ypos);
		}
		LCD_Erase_Square(Xpos,Ypos-1);
	}
	if (orientation == 2){
			for (int i = Ypos; i < (Ypos+3); i++){
				LCD_Erase_Square(Xpos+1,i);
			}
			LCD_Erase_Square(Xpos+2,Ypos);
		}
	if (orientation == 3){
		for (int i = Xpos; i < (Xpos+3); i++){
			LCD_Erase_Square(i,Ypos);
		}
		LCD_Erase_Square(Xpos+2,Ypos+1);
	}
	if (orientation == 4){
		for (int i = Ypos-1; i < (Ypos+2); i++){
			LCD_Erase_Square(Xpos+1,i);
		}
		LCD_Erase_Square(Xpos,Ypos+1);
	}
}

void drawBlockThree(uint16_t Xpos, uint16_t Ypos, uint16_t orientation){
	if (orientation == 1){
		for (int i = Xpos; i < (Xpos+3); i++){
			LCD_Draw_Square_Fill_Border(i,Ypos,LCD_COLOR_ORANGE);
		}
		LCD_Draw_Square_Fill_Border(Xpos+2,Ypos-1,LCD_COLOR_ORANGE);
	}
	if (orientation == 2){
			for (int i = Ypos; i < (Ypos+3); i++){
				LCD_Draw_Square_Fill_Border(Xpos+1,i,LCD_COLOR_ORANGE);
			}
			LCD_Draw_Square_Fill_Border(Xpos+2,Ypos,LCD_COLOR_ORANGE);
		}
	if (orientation == 3){
		for (int i = Xpos; i < (Xpos+3); i++){
			LCD_Draw_Square_Fill_Border(i,Ypos,LCD_COLOR_ORANGE);
		}
		LCD_Draw_Square_Fill_Border(Xpos,Ypos+1,LCD_COLOR_ORANGE);
	}
	if (orientation == 4){
		for (int i = Ypos-1; i < (Ypos+2); i++){
			LCD_Draw_Square_Fill_Border(Xpos+1,i,LCD_COLOR_ORANGE);
		}
		LCD_Draw_Square_Fill_Border(Xpos,Ypos-1,LCD_COLOR_ORANGE);
	}
}

void eraseBlockThree(uint16_t Xpos, uint16_t Ypos, uint16_t orientation){
	if (orientation == 1){
		for (int i = Xpos; i < (Xpos+3); i++){
			LCD_Erase_Square(i,Ypos);
		}
		LCD_Erase_Square(Xpos+2,Ypos-1);
	}
	if (orientation == 2){
			for (int i = Ypos; i < (Ypos+3); i++){
				LCD_Erase_Square(Xpos+1,i);
			}
			LCD_Erase_Square(Xpos+2,Ypos);
		}
	if (orientation == 3){
		for (int i = Xpos; i < (Xpos+3); i++){
			LCD_Erase_Square(i,Ypos);
		}
		LCD_Erase_Square(Xpos,Ypos+1);
	}
	if (orientation == 4){
		for (int i = Ypos-1; i < (Ypos+2); i++){
			LCD_Erase_Square(Xpos+1,i);
		}
		LCD_Erase_Square(Xpos,Ypos-1);
	}
}

void drawBlockFour(uint16_t Xpos, uint16_t Ypos, uint16_t orientation){
	for (int i = Xpos; i < (Xpos+2); i++){
			LCD_Draw_Square_Fill_Border(i,Ypos,LCD_COLOR_YELLOW);
			LCD_Draw_Square_Fill_Border(i,Ypos-1,LCD_COLOR_YELLOW);
		}
}

void eraseBlockFour(uint16_t Xpos, uint16_t Ypos, uint16_t orientation){
	for (int i = Xpos; i < (Xpos+2); i++){
			LCD_Erase_Square(i,Ypos);
			LCD_Erase_Square(i,Ypos-1);
		}
}

void drawBlockFive(uint16_t Xpos, uint16_t Ypos, uint16_t orientation){
	if (orientation == 1 || orientation == 3){
		for (int i = Xpos; i < (Xpos+2); i++){
			LCD_Draw_Square_Fill_Border(i,Ypos,LCD_COLOR_GREEN);
			LCD_Draw_Square_Fill_Border(i+1,Ypos-1,LCD_COLOR_GREEN);
		}
	}
	if (orientation == 2 || orientation == 4){
		for (int i = Ypos; i < (Ypos+2); i++){
			LCD_Draw_Square_Fill_Border(Xpos,i-1,LCD_COLOR_GREEN);
			LCD_Draw_Square_Fill_Border(Xpos+1,i,LCD_COLOR_GREEN);
		}
	}
}

void eraseBlockFive(uint16_t Xpos, uint16_t Ypos, uint16_t orientation){
	if (orientation == 1 || orientation == 3){
		for (int i = Xpos; i < (Xpos+2); i++){
			LCD_Erase_Square(i,Ypos);
			LCD_Erase_Square(i+1,Ypos-1);
		}
	}
	if (orientation == 2 || orientation == 4){
		for (int i = Ypos; i < (Ypos+2); i++){
			LCD_Erase_Square(Xpos,i-1);
			LCD_Erase_Square(Xpos+1,i);
		}
	}
}

void drawBlockSix(uint16_t Xpos, uint16_t Ypos, uint16_t orientation){
	if (orientation == 1){
		for (int i = Xpos-1; i < (Xpos+2); i++){
			LCD_Draw_Square_Fill_Border(i,Ypos,LCD_COLOR_MAGENTA);
		}
		LCD_Draw_Square_Fill_Border(Xpos,Ypos-1,LCD_COLOR_MAGENTA);
	}

	if (orientation == 3){
		for (int i = Xpos-1; i < (Xpos+2); i++){
			LCD_Draw_Square_Fill_Border(i,Ypos,LCD_COLOR_MAGENTA);
		}
		LCD_Draw_Square_Fill_Border(Xpos,Ypos+1,LCD_COLOR_MAGENTA);
	}
	if (orientation == 2){
		for (int i = Ypos-1; i < (Ypos+2); i++){
			LCD_Draw_Square_Fill_Border(Xpos,i,LCD_COLOR_MAGENTA);
		}
		LCD_Draw_Square_Fill_Border(Xpos+1,Ypos,LCD_COLOR_MAGENTA);
	}
	if (orientation == 4){
		for (int i = Ypos-1; i < (Ypos+2); i++){
			LCD_Draw_Square_Fill_Border(Xpos,i,LCD_COLOR_MAGENTA);
		}
		LCD_Draw_Square_Fill_Border(Xpos-1,Ypos,LCD_COLOR_MAGENTA);
	}
}

void eraseBlockSix(uint16_t Xpos, uint16_t Ypos, uint16_t orientation){
	if (orientation == 1){
		for (int i = Xpos-1; i < (Xpos+2); i++){
			LCD_Erase_Square(i,Ypos);
		}
		LCD_Erase_Square(Xpos,Ypos-1);
	}

	if (orientation == 3){
		for (int i = Xpos-1; i < (Xpos+2); i++){
			LCD_Erase_Square(i,Ypos);
		}
		LCD_Erase_Square(Xpos,Ypos+1);
	}
	if (orientation == 2){
		for (int i = Ypos-1; i < (Ypos+2); i++){
			LCD_Erase_Square(Xpos,i);
		}
		LCD_Erase_Square(Xpos+1,Ypos);
	}
	if (orientation == 4){
		for (int i = Ypos-1; i < (Ypos+2); i++){
			LCD_Erase_Square(Xpos,i);
		}
		LCD_Erase_Square(Xpos-1,Ypos);
	}
}

void drawBlockSeven(uint16_t Xpos, uint16_t Ypos, uint16_t orientation){
	if (orientation == 1 || orientation == 3){
		for (int i = Xpos; i < (Xpos+2); i++){
			LCD_Draw_Square_Fill_Border(i,Ypos,LCD_COLOR_RED);
			LCD_Draw_Square_Fill_Border(i-1,Ypos-1,LCD_COLOR_RED);
		}
	}
	if (orientation == 2 || orientation == 4){
		for (int i = Ypos; i < (Ypos+2); i++){
			LCD_Draw_Square_Fill_Border(Xpos,i,LCD_COLOR_RED);
			LCD_Draw_Square_Fill_Border(Xpos+1,i-1,LCD_COLOR_RED);
		}
	}
}

void eraseBlockSeven(uint16_t Xpos, uint16_t Ypos, uint16_t orientation){
	if (orientation == 1 || orientation == 3){
		for (int i = Xpos; i < (Xpos+2); i++){
			LCD_Erase_Square(i,Ypos);
			LCD_Erase_Square(i-1,Ypos-1);
		}
	}
	if (orientation == 2 || orientation == 4){
		for (int i = Ypos; i < (Ypos+2); i++){
			LCD_Erase_Square(Xpos,i);
			LCD_Erase_Square(Xpos+1,i-1);
		}
	}
}

void drawScreen(){
		for (int i = 0; i < 16; i++)
		{
			for (int j = 0; j < 12; j++)
			{
				if (LCD_Screen[i][j] != 0)
				{
					uint16_t color = Obj_Color[i][j];
					LCD_Draw_Square_Fill_Border(i,j,color);
				}
			}
		}
}

void rotateBlock(){
	uint16_t BlockNum = CurrentBlock[0];
	uint16_t Xpos = CurrentBlock[1];
	uint16_t Ypos = CurrentBlock[2];
	uint16_t OrgOrientation = CurrentBlock[3];
	uint16_t NewOrientation;
	if (OrgOrientation < 4){
		NewOrientation = OrgOrientation + 1;
	}
	else if (OrgOrientation >= 4){
		NewOrientation = 1;
	}

	if (BlockNum == 1){
		eraseBlockOne(Xpos, Ypos, OrgOrientation);
		drawBlockOne(Xpos, Ypos, NewOrientation);
		updateCurrentBlock(1, Xpos, Ypos, NewOrientation);
	}
	if (BlockNum == 2){
		eraseBlockTwo(Xpos, Ypos, OrgOrientation);
		drawBlockTwo(Xpos, Ypos, NewOrientation);
		updateCurrentBlock(2, Xpos, Ypos, NewOrientation);
		}
	if (BlockNum == 3){
		eraseBlockThree(Xpos, Ypos, OrgOrientation);
		drawBlockThree(Xpos, Ypos, NewOrientation);
		updateCurrentBlock(3, Xpos, Ypos, NewOrientation);
		}
	if (BlockNum == 4){
		eraseBlockFour(Xpos, Ypos, OrgOrientation);
		drawBlockFour(Xpos, Ypos, NewOrientation);
		updateCurrentBlock(4, Xpos, Ypos, NewOrientation);
		}
	if (BlockNum == 5){
		eraseBlockFive(Xpos, Ypos, OrgOrientation);
		drawBlockFive(Xpos, Ypos, NewOrientation);
		updateCurrentBlock(5, Xpos, Ypos, NewOrientation);
		}
	if (BlockNum == 6){
		eraseBlockSix(Xpos, Ypos, OrgOrientation);
		drawBlockSix(Xpos, Ypos, NewOrientation);
		updateCurrentBlock(6, Xpos, Ypos, NewOrientation);
		}
	if (BlockNum == 7){
		eraseBlockSeven(Xpos, Ypos, OrgOrientation);
		drawBlockSeven(Xpos, Ypos, NewOrientation);
		updateCurrentBlock(7, Xpos, Ypos, NewOrientation);
		}
}

void updateCurrentBlock(uint16_t BlockNum, uint16_t Xpos, uint16_t Ypos, uint16_t Orientation){
	CurrentBlock[0] = BlockNum;
	CurrentBlock[1] = Xpos;
	CurrentBlock[2] = Ypos;
	CurrentBlock[3] = Orientation;
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void LCD_Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

// Touch Functionality   //

#if COMPILE_TOUCH_FUNCTIONS == 1

void InitializeLCDTouch(void)
{
  if(STMPE811_Init() != STMPE811_State_Ok)
  {
	 for(;;); // Hang code due to error in initialzation
  }
}

STMPE811_State_t returnTouchStateAndLocation(STMPE811_TouchData * touchStruct)
{
	return STMPE811_ReadTouch(touchStruct);
}

void DetermineTouchPosition(STMPE811_TouchData * touchStruct)
{
	STMPE811_DetermineTouchPosition(touchStruct);
}

uint8_t ReadRegisterFromTouchModule(uint8_t RegToRead)
{
	return STMPE811_Read(RegToRead);
}

void WriteDataToTouchModule(uint8_t RegToWrite, uint8_t writeData)
{
	STMPE811_Write(RegToWrite, writeData);
}



#endif // COMPILE_TOUCH_FUNCTIONS
