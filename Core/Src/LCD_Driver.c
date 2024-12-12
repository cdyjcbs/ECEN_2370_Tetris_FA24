/*
 * LCD_Driver.c
 *
 *  Created on: Sep 28, 2023
 *      Author: Xavion
 *      Modified by: Cody Jacobs
 *      Tetris - Button Driver Source
 *		Date: 12/11/2024
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
int LCD_Screen[16][12] = {0};
int blockSize = 20;
int topRow[12] = {0};
int result[5] = {0};
uint16_t gameTime = 0;
int finalGameTime[3] = {0};


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

// Modification of Draw_Circle_Fill to make Squares instead
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

// Modification of Draw_Square_Fill to include a black border
void LCD_Draw_Square_Fill_Border(uint16_t Xpos, uint16_t Ypos, uint16_t color)
{
	uint16_t x = Xpos*blockSize;
	uint16_t y = Ypos*blockSize;
	for(int16_t i=1; i<=blockSize; i++)
	    {
	        for(int16_t j=1; j<=blockSize; j++)
	        {
	            LCD_Draw_Pixel(i+x, j+y, color);

	            if (color != GameScreenColor){
					LCD_Draw_Vertical_Line(x,y,blockSize,LCD_COLOR_BLACK);
					LCD_Draw_Vertical_Line(x+blockSize,y,blockSize+1,LCD_COLOR_BLACK);
					LCD_Draw_Horizontal_Line(x,y,blockSize,LCD_COLOR_BLACK);
					LCD_Draw_Horizontal_Line(x,y+blockSize,blockSize,LCD_COLOR_BLACK);
	            }
	            else if (color == GameScreenColor){
					LCD_Draw_Vertical_Line(x,y,blockSize,GameScreenColor);
					LCD_Draw_Vertical_Line(x+blockSize,y,blockSize+1,GameScreenColor);
					LCD_Draw_Horizontal_Line(x,y,blockSize,GameScreenColor);
					LCD_Draw_Horizontal_Line(x,y+blockSize,blockSize,GameScreenColor);
	            }
	        }
	    }
}

// Erase drawn squares by re-drawing them and their borders as the color of the screen
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

// Function to draw Welcome Screen
void GameInit(void)
{

	// Initialize topRow to the bottom row
	for (int i = 0; i < 12; i++){
		topRow[i] = 15;
	}

	// Initialize the Obj_Color array to the GameScreenColor
	for (int j = 0; j < 16; j++){
		for (int k = 0; k < 12; k++){
			Obj_Color[j][k] = GameScreenColor;
		}
	}

	// Write welcome message in middle of screen
	LCD_Clear(0,GameScreenColor);
	LCD_SetTextColor(LCD_COLOR_BLACK);
	LCD_SetFont(&Font16x24);

	LCD_DisplayChar(80,130,'W');
	LCD_DisplayChar(94,130,'e');
	LCD_DisplayChar(104,130,'l');
	LCD_DisplayChar(112,130,'c');
	LCD_DisplayChar(124,130,'o');
	LCD_DisplayChar(140,130,'m');
	LCD_DisplayChar(155,130,'e');

	LCD_DisplayChar(110,155,'t');
	LCD_DisplayChar(120,155,'o');

	LCD_DisplayChar(95,180,'T');
	LCD_DisplayChar(105,180,'e');
	LCD_DisplayChar(115,180,'t');
	LCD_DisplayChar(125,180,'r');
	LCD_DisplayChar(132,180,'i');
	LCD_DisplayChar(139,180,'s');

	// Display blocks on size 10 at top of Welcome Screen
	blockSize = 10;
	drawBlockOne(4.5,2.5,1);
	drawBlockTwo(9.5,2.5,1);
	drawBlockThree(13.5,2.5,1);
	drawBlockFour(17.5,2.5,1);
	drawBlockFive(6.5,5.5,1);
	drawBlockSix(10.5,5.5,1);
	drawBlockSeven(14.5,5.5,1);

	// Display START message at bottom of screen
	LCD_DisplayChar(50,270,'T');
	LCD_DisplayChar(62,270,'o');
	LCD_DisplayChar(74,270,'u');
	LCD_DisplayChar(86,270,'c');
	LCD_DisplayChar(97,270,'h');

	LCD_DisplayChar(117,270,'s');
	LCD_DisplayChar(128,270,'c');
	LCD_DisplayChar(139,270,'r');
	LCD_DisplayChar(148,270,'e');
	LCD_DisplayChar(160,270,'e');
	LCD_DisplayChar(172,270,'n');

	LCD_DisplayChar(40,295,'t');
	LCD_DisplayChar(50,295,'o');

	LCD_DisplayChar(72,295,'S');
	LCD_DisplayChar(87,295,'T');
	LCD_DisplayChar(101,295,'A');
	LCD_DisplayChar(117,295,'R');
	LCD_DisplayChar(132,295,'T');

	LCD_DisplayChar(149,295,'g');
	LCD_DisplayChar(161,295,'a');
	LCD_DisplayChar(175,295,'m');
	LCD_DisplayChar(189,295,'e');
}

// Function to draw game screen and first block
void gameStart(){
	screenReset();
	HAL_Delay(1000);

	blockSize = 20;
	LCD_Clear(0,GameScreenColor);
	LCD_SetTextColor(LCD_COLOR_WHITE);
	LCD_SetFont(&Font16x24);

	drawBottomBorder();
	uint32_t randBlock = GetRandomBlock();
	updateCurrentBlock(randBlock, 5, 1, 1);	drawCurrentBlock();
	HAL_Delay(100);
}

// Function to reset Obj_Color array to GameScreenColor and LCD_Screen array to 0
void screenReset(){
	for (uint16_t i = 0; i < 16; i++){
		for (uint16_t j = 0; j < 12; j++){
			Obj_Color[i][j] = GameScreenColor;
			LCD_Screen[i][j] = 0;
		}
	}
}

// Game over screen displaying game time and lines cleared
void gameOver(){
		totalGameTime();
		LCD_Clear(0,LCD_COLOR_BLACK);
		LCD_SetTextColor(LCD_COLOR_RED);
		LCD_SetFont(&Font16x24);

		LCD_DisplayChar(55,20,'G');
		LCD_DisplayChar(71,20,'a');
		LCD_DisplayChar(85,20,'m');
		LCD_DisplayChar(100,20,'e');

		LCD_DisplayChar(120,20,'O');
		LCD_DisplayChar(137,20,'v');
		LCD_DisplayChar(150,20,'e');
		LCD_DisplayChar(160,20,'r');
		LCD_DisplayChar(167,20,'!');
		LCD_DisplayChar(172,20,'!');
		LCD_DisplayChar(177,20,'!');

		LCD_DisplayChar(55,50,'G');
		LCD_DisplayChar(71,50,'a');
		LCD_DisplayChar(85,50,'m');
		LCD_DisplayChar(100,50,'e');

		LCD_DisplayChar(120,50,'T');
		LCD_DisplayChar(132,50,'i');
		LCD_DisplayChar(142,50,'m');
		LCD_DisplayChar(157,50,'e');
		LCD_DisplayChar(167,50,':');

		LCD_DisplayChar(95,80,(finalGameTime[0] + '0'));
		LCD_DisplayChar(105,80,':');
		LCD_DisplayChar(115,80,(finalGameTime[1] + '0'));
		LCD_DisplayChar(128,80,(finalGameTime[2] + '0'));

		LCD_DisplayChar(82,145,'S');
		LCD_DisplayChar(97,145,'i');
		LCD_DisplayChar(108,145,'n');
		LCD_DisplayChar(121,145,'g');
		LCD_DisplayChar(131,145,'l');
		LCD_DisplayChar(141,145,'e');
		LCD_DisplayChar(151,145,':');
		LCD_DisplayChar(163,146,(result[1] + '0'));

		LCD_DisplayChar(82,170,'D');
		LCD_DisplayChar(97,170,'o');
		LCD_DisplayChar(108,170,'u');
		LCD_DisplayChar(121,170,'b');
		LCD_DisplayChar(131,170,'l');
		LCD_DisplayChar(141,170,'e');
		LCD_DisplayChar(151,170,':');
		LCD_DisplayChar(163,171,(result[2] + '0'));

		LCD_DisplayChar(82,195,'T');
		LCD_DisplayChar(97,195,'r');
		LCD_DisplayChar(108,195,'i');
		LCD_DisplayChar(119,195,'p');
		LCD_DisplayChar(131,195,'l');
		LCD_DisplayChar(141,195,'e');
		LCD_DisplayChar(151,195,':');
		LCD_DisplayChar(163,196,(result[3] + '0'));

		LCD_DisplayChar(82,220,'T');
		LCD_DisplayChar(97,220,'e');
		LCD_DisplayChar(108,220,'t');
		LCD_DisplayChar(119,220,'r');
		LCD_DisplayChar(131,220,'i');
		LCD_DisplayChar(141,220,'s');
		LCD_DisplayChar(151,220,':');
		LCD_DisplayChar(163,221,(result[4] + '0'));
		HAL_Delay(1000);
}

// Break total game time into minutes, 10's seconds, and 1's seconds
void totalGameTime(){
	uint16_t endGameTime = gameTime;
	if (endGameTime < 60){
		int firstDigit = 0;
		finalGameTime[0] = firstDigit;

		int secondDigit = (int)endGameTime;
		secondDigit = secondDigit/10;
		finalGameTime[1] = secondDigit;

		int thirdDigit = (int)endGameTime;
		thirdDigit = thirdDigit % 10;
		finalGameTime[2] = thirdDigit;
	}

	else if (endGameTime >= 60){
		uint16_t firstDigit = endGameTime;
		firstDigit = firstDigit/60;
		finalGameTime[0] = firstDigit;

		int secondDigit = endGameTime-(firstDigit*60);
		int thirdDigit = secondDigit;

		secondDigit = secondDigit/10;
		finalGameTime[1] = secondDigit;

		thirdDigit = thirdDigit % 10;
		finalGameTime[2] = thirdDigit;
	}

}

// Draw Block One at X, Y, and in given orientation
void drawBlockOne(uint16_t Xpos, uint16_t Ypos, uint16_t orientation){
	if (orientation == 1 || orientation == 3){
		for (int i = Xpos; i < (Xpos+4); i++){
			LCD_Draw_Square_Fill_Border(i,Ypos,LCD_COLOR_CYAN);
			LCD_Screen[Ypos][i] = 1;
			Obj_Color[Ypos][i] = LCD_COLOR_CYAN;
		}
	}
	if (orientation == 2 || orientation == 4){
		for (int i = Ypos-1; i < (Ypos+3); i++){
			LCD_Draw_Square_Fill_Border(Xpos,i,LCD_COLOR_CYAN);
			LCD_Screen[i][Xpos] = 1;
			Obj_Color[i][Xpos] = LCD_COLOR_CYAN;
		}
	}
}

// Erase Block One at X, Y, and in given orientation
void eraseBlockOne(uint16_t Xpos, uint16_t Ypos, uint16_t orientation){
	if (orientation == 1 || orientation == 3){
		for (int i = Xpos; i < (Xpos+4); i++){
			LCD_Erase_Square(i, Ypos);
			LCD_Screen[Ypos][i] = 0;
			Obj_Color[Ypos][i] = GameScreenColor;
		}
	}
	if (orientation == 2 || orientation == 4){
		for (int i = Ypos-1; i < (Ypos+3); i++){
			LCD_Erase_Square(Xpos, i);
			LCD_Screen[i][Xpos] = 0;
			Obj_Color[i][Xpos] = GameScreenColor;
		}
	}
}

// Draw Block Two at X, Y, and in given orientation
void drawBlockTwo(uint16_t Xpos, uint16_t Ypos, uint16_t orientation){
	if (orientation == 1){
		for (int i = Xpos; i < (Xpos+3); i++){
			LCD_Draw_Square_Fill_Border(i,Ypos,LCD_COLOR_BLUE);
			LCD_Screen[Ypos][i] = 1;
			Obj_Color[Ypos][i] = LCD_COLOR_BLUE;
		}
		LCD_Draw_Square_Fill_Border(Xpos,Ypos-1,LCD_COLOR_BLUE);
		LCD_Screen[Ypos-1][Xpos] = 1;
		Obj_Color[Ypos-1][Xpos] = LCD_COLOR_BLUE;
	}
	if (orientation == 2){
			for (int i = Ypos-1; i < (Ypos+2); i++){
				LCD_Draw_Square_Fill_Border(Xpos,i,LCD_COLOR_BLUE);
				LCD_Screen[i][Xpos] = 1;
				Obj_Color[i][Xpos] = LCD_COLOR_BLUE;
			}
			LCD_Draw_Square_Fill_Border(Xpos+1,Ypos-1,LCD_COLOR_BLUE);
			LCD_Screen[Ypos-1][Xpos+1] = 1;
			Obj_Color[Ypos-1][Xpos+1] = LCD_COLOR_BLUE;
		}
	if (orientation == 3){
		for (int i = Xpos; i < (Xpos+3); i++){
			LCD_Draw_Square_Fill_Border(i,Ypos,LCD_COLOR_BLUE);
			LCD_Screen[Ypos][i] = 1;
			Obj_Color[Ypos][i] = LCD_COLOR_BLUE;
		}
		LCD_Draw_Square_Fill_Border(Xpos+2,Ypos+1,LCD_COLOR_BLUE);
		LCD_Screen[Ypos+1][Xpos+2] = 1;
		Obj_Color[Ypos+1][Xpos+2] = LCD_COLOR_BLUE;
	}
	if (orientation == 4){
		for (int i = Ypos-1; i < (Ypos+2); i++){
			LCD_Draw_Square_Fill_Border(Xpos+1,i,LCD_COLOR_BLUE);
			LCD_Screen[i][Xpos+1] = 1;
			Obj_Color[i][Xpos+1] = LCD_COLOR_BLUE;
		}
		LCD_Draw_Square_Fill_Border(Xpos,Ypos+1,LCD_COLOR_BLUE);
		LCD_Screen[Ypos+1][Xpos] = 1;
		Obj_Color[Ypos+1][Xpos] = LCD_COLOR_BLUE;
	}
}

// Erase Block Two at X, Y, and in given orientation
void eraseBlockTwo(uint16_t Xpos, uint16_t Ypos, uint16_t orientation){
	if (orientation == 1){
		for (int i = Xpos; i < (Xpos+3); i++){
			LCD_Erase_Square(i,Ypos);
			LCD_Screen[Ypos][i] = 0;
			Obj_Color[Ypos][i] = GameScreenColor;
		}
		LCD_Erase_Square(Xpos,Ypos-1);
		LCD_Screen[Ypos-1][Xpos] = 0;
		Obj_Color[Ypos-1][Xpos] = GameScreenColor;
	}
	if (orientation == 2){
			for (int i = Ypos-1; i < (Ypos+2); i++){
				LCD_Erase_Square(Xpos,i);
				LCD_Screen[i][Xpos] = 0;
				Obj_Color[i][Xpos] = GameScreenColor;
			}
			LCD_Erase_Square(Xpos+1,Ypos-1);
			LCD_Screen[Ypos-1][Xpos+1] = 0;
			Obj_Color[Ypos-1][Xpos+1] = GameScreenColor;
		}
	if (orientation == 3){
		for (int i = Xpos; i < (Xpos+3); i++){
			LCD_Erase_Square(i,Ypos);
			LCD_Screen[Ypos][i] = 0;
			Obj_Color[Ypos][i] = GameScreenColor;
		}
		LCD_Erase_Square(Xpos+2,Ypos+1);
		LCD_Screen[Ypos+1][Xpos+2] = 0;
		Obj_Color[Ypos+1][Xpos+2] = GameScreenColor;
	}
	if (orientation == 4){
		for (int i = Ypos-1; i < (Ypos+2); i++){
			LCD_Erase_Square(Xpos+1,i);
			LCD_Screen[i][Xpos+1] = 0;
			Obj_Color[i][Xpos+1] = GameScreenColor;
		}
		LCD_Erase_Square(Xpos,Ypos+1);
		LCD_Screen[Ypos+1][Xpos] = 0;
		Obj_Color[Ypos+1][Xpos] = GameScreenColor;
	}
}

// Draw Block Three at X, Y, and in given orientation
void drawBlockThree(uint16_t Xpos, uint16_t Ypos, uint16_t orientation){
	if (orientation == 1){
		for (int i = Xpos; i < (Xpos+3); i++){
			LCD_Draw_Square_Fill_Border(i,Ypos,LCD_COLOR_ORANGE);
			LCD_Screen[Ypos][i] = 1;
			Obj_Color[Ypos][i] = LCD_COLOR_ORANGE;
		}
		LCD_Draw_Square_Fill_Border(Xpos+2,Ypos-1,LCD_COLOR_ORANGE);
		LCD_Screen[Ypos-1][Xpos+2] = 1;
		Obj_Color[Ypos-1][Xpos+2] = LCD_COLOR_ORANGE;
	}
	if (orientation == 2){
			for (int i = Ypos-1; i < (Ypos+2); i++){
				LCD_Draw_Square_Fill_Border(Xpos,i,LCD_COLOR_ORANGE);
				LCD_Screen[i][Xpos] = 1;
				Obj_Color[i][Xpos] = LCD_COLOR_ORANGE;
			}
			LCD_Draw_Square_Fill_Border(Xpos+1,Ypos+1,LCD_COLOR_ORANGE);
			LCD_Screen[Ypos+1][Xpos+1] = 1;
			Obj_Color[Ypos+1][Xpos+1] = LCD_COLOR_ORANGE;
		}
	if (orientation == 3){
		for (int i = Xpos; i < (Xpos+3); i++){
			LCD_Draw_Square_Fill_Border(i,Ypos,LCD_COLOR_ORANGE);
			LCD_Screen[Ypos][i] = 1;
			Obj_Color[Ypos][i] = LCD_COLOR_ORANGE;
		}
		LCD_Draw_Square_Fill_Border(Xpos,Ypos+1,LCD_COLOR_ORANGE);
		LCD_Screen[Ypos+1][Xpos] = 1;
		Obj_Color[Ypos+1][Xpos] = LCD_COLOR_ORANGE;
	}
	if (orientation == 4){
		for (int i = Ypos-1; i < (Ypos+2); i++){
			LCD_Draw_Square_Fill_Border(Xpos+1,i,LCD_COLOR_ORANGE);
			LCD_Screen[i][Xpos+1] = 1;
			Obj_Color[i][Xpos+1] = LCD_COLOR_ORANGE;
		}
		LCD_Draw_Square_Fill_Border(Xpos,Ypos-1,LCD_COLOR_ORANGE);
		LCD_Screen[Ypos-1][Xpos] = 1;
		Obj_Color[Ypos-1][Xpos] = LCD_COLOR_ORANGE;
	}
}

// Erase Block Three at X, Y, and in given orientation
void eraseBlockThree(uint16_t Xpos, uint16_t Ypos, uint16_t orientation){
	if (orientation == 1){
		for (int i = Xpos; i < (Xpos+3); i++){
			LCD_Erase_Square(i,Ypos);
			LCD_Screen[Ypos][i] = 0;
			Obj_Color[Ypos][i] = GameScreenColor;
		}
		LCD_Erase_Square(Xpos+2,Ypos-1);
		LCD_Screen[Ypos-1][Xpos+2] = 0;
		Obj_Color[Ypos-1][Xpos+2] = GameScreenColor;
	}
	if (orientation == 2){
			for (int i = Ypos-1; i < (Ypos+2); i++){
				LCD_Erase_Square(Xpos,i);
				LCD_Screen[i][Xpos] = 0;
				Obj_Color[i][Xpos] = GameScreenColor;
			}
			LCD_Erase_Square(Xpos+1,Ypos+1);
			LCD_Screen[Ypos+1][Xpos+1] = 0;
			Obj_Color[Ypos+1][Xpos+1] = GameScreenColor;
		}
	if (orientation == 3){
		for (int i = Xpos; i < (Xpos+3); i++){
			LCD_Erase_Square(i,Ypos);
			LCD_Screen[Ypos][i] = 0;
			Obj_Color[Ypos][i] = GameScreenColor;
		}
		LCD_Erase_Square(Xpos,Ypos+1);
		LCD_Screen[Ypos+1][Xpos] = 0;
		Obj_Color[Ypos+1][Xpos] = GameScreenColor;
	}
	if (orientation == 4){
		for (int i = Ypos-1; i < (Ypos+2); i++){
			LCD_Erase_Square(Xpos+1,i);
			LCD_Screen[i][Xpos+1] = 0;
			Obj_Color[i][Xpos+1] = GameScreenColor;
		}
		LCD_Erase_Square(Xpos,Ypos-1);
		LCD_Screen[Ypos-1][Xpos] = 0;
		Obj_Color[Ypos-1][Xpos] = GameScreenColor;
	}
}

// Draw Block Four at X, Y, and in given orientation
void drawBlockFour(uint16_t Xpos, uint16_t Ypos, uint16_t orientation){
	for (int i = Xpos; i < (Xpos+2); i++){
			LCD_Draw_Square_Fill_Border(i,Ypos,LCD_COLOR_YELLOW);
			LCD_Screen[Ypos][i] = 1;
			Obj_Color[Ypos][i] = LCD_COLOR_YELLOW;
			LCD_Draw_Square_Fill_Border(i,Ypos-1,LCD_COLOR_YELLOW);
			LCD_Screen[Ypos-1][i] = 1;
			Obj_Color[Ypos-1][i] = LCD_COLOR_YELLOW;
		}
}

// Erase Block Four at X, Y, and in given orientation
void eraseBlockFour(uint16_t Xpos, uint16_t Ypos, uint16_t orientation){
	for (int i = Xpos; i < (Xpos+2); i++){
			LCD_Erase_Square(i,Ypos);
			LCD_Screen[Ypos][i] = 0;
			Obj_Color[Ypos][i] = GameScreenColor;
			LCD_Erase_Square(i,Ypos-1);
			LCD_Screen[Ypos-1][i] = 0;
			Obj_Color[Ypos-1][i] = GameScreenColor;
		}
}

// Draw Block Five at X, Y, and in given orientation
void drawBlockFive(uint16_t Xpos, uint16_t Ypos, uint16_t orientation){
	if (orientation == 1 || orientation == 3){
		for (int i = Xpos; i < (Xpos+2); i++){
			LCD_Draw_Square_Fill_Border(i,Ypos,LCD_COLOR_GREEN);
			LCD_Screen[Ypos][i] = 1;
			Obj_Color[Ypos][i] = LCD_COLOR_GREEN;
			LCD_Draw_Square_Fill_Border(i+1,Ypos-1,LCD_COLOR_GREEN);
			LCD_Screen[Ypos-1][i+1] = 1;
			Obj_Color[Ypos-1][i+1] = LCD_COLOR_GREEN;
		}
	}
	// Y-1			1	1
	//   Y	    1	1
	// Y+1
	// Y+2
	//	   X-1  X  X+1  X+2
	if (orientation == 2 || orientation == 4){
		for (int i = Ypos; i < (Ypos+2); i++){
			LCD_Draw_Square_Fill_Border(Xpos,i-1,LCD_COLOR_GREEN);
			LCD_Screen[i-1][Xpos] = 1;
			Obj_Color[i-1][Xpos] = LCD_COLOR_GREEN;
			LCD_Draw_Square_Fill_Border(Xpos+1,i,LCD_COLOR_GREEN);
			LCD_Screen[i][Xpos+1] = 1;
			Obj_Color[i][Xpos+1] = LCD_COLOR_GREEN;
		}
	}
	// Y-1		1
	//   Y	    1	1
	// Y+1			1
	// Y+2
	//	   X-1  X  X+1  X+2
}

// Erase Block Five at X, Y, and in given orientation
void eraseBlockFive(uint16_t Xpos, uint16_t Ypos, uint16_t orientation){
	if (orientation == 1 || orientation == 3){
		for (int i = Xpos; i < (Xpos+2); i++){
			LCD_Erase_Square(i,Ypos);
			LCD_Screen[Ypos][i] = 0;
			Obj_Color[Ypos][i] = GameScreenColor;
			LCD_Erase_Square(i+1,Ypos-1);
			LCD_Screen[Ypos-1][i+1] = 0;
			Obj_Color[Ypos-1][i+1] = GameScreenColor;
		}
	}
	if (orientation == 2 || orientation == 4){
		for (int i = Ypos; i < (Ypos+2); i++){
			LCD_Erase_Square(Xpos,i-1);
			LCD_Screen[i-1][Xpos] = 0;
			Obj_Color[i-1][Xpos] = GameScreenColor;
			LCD_Erase_Square(Xpos+1,i);
			LCD_Screen[i][Xpos+1] = 0;
			Obj_Color[i][Xpos+1] = GameScreenColor;
		}
	}
}

// Draw Block Six at X, Y, and in given orientation
void drawBlockSix(uint16_t Xpos, uint16_t Ypos, uint16_t orientation){
	if (orientation == 1){
		for (int i = Xpos; i < (Xpos+3); i++){
			LCD_Draw_Square_Fill_Border(i,Ypos,LCD_COLOR_MAGENTA);
			LCD_Screen[Ypos][i] = 1;
			Obj_Color[Ypos][i] = LCD_COLOR_MAGENTA;
		}
		LCD_Draw_Square_Fill_Border(Xpos+1,Ypos-1,LCD_COLOR_MAGENTA);
		LCD_Screen[Ypos-1][Xpos+1] = 1;
		Obj_Color[Ypos-1][Xpos+1] = LCD_COLOR_MAGENTA;
	}

	if (orientation == 3){
		for (int i = Xpos; i < (Xpos+3); i++){
			LCD_Draw_Square_Fill_Border(i,Ypos,LCD_COLOR_MAGENTA);
			LCD_Screen[Ypos][i] = 1;
			Obj_Color[Ypos][i] = LCD_COLOR_MAGENTA;
		}
		LCD_Draw_Square_Fill_Border(Xpos+1,Ypos+1,LCD_COLOR_MAGENTA);
		LCD_Screen[Ypos+1][Xpos+1] = 1;
		Obj_Color[Ypos+1][Xpos+1] = LCD_COLOR_MAGENTA;
	}
	if (orientation == 2){
		for (int i = Ypos-1; i < (Ypos+2); i++){
			LCD_Draw_Square_Fill_Border(Xpos,i,LCD_COLOR_MAGENTA);
			LCD_Screen[i][Xpos] = 1;
			Obj_Color[i][Xpos] = LCD_COLOR_MAGENTA;
		}
		LCD_Draw_Square_Fill_Border(Xpos+1,Ypos,LCD_COLOR_MAGENTA);
		LCD_Screen[Ypos][Xpos+1] = 1;
		Obj_Color[Ypos][Xpos+1] = LCD_COLOR_MAGENTA;
	}
	if (orientation == 4){
		for (int i = Ypos-1; i < (Ypos+2); i++){
			LCD_Draw_Square_Fill_Border(Xpos+1,i,LCD_COLOR_MAGENTA);
			LCD_Screen[i][Xpos+1] = 1;
			Obj_Color[i][Xpos+1] = LCD_COLOR_MAGENTA;
		}
		LCD_Draw_Square_Fill_Border(Xpos,Ypos,LCD_COLOR_MAGENTA);
		LCD_Screen[Ypos][Xpos] = 1;
		Obj_Color[Ypos][Xpos] = LCD_COLOR_MAGENTA;
	}
}

// Erase Block Six at X, Y, and in given orientation
void eraseBlockSix(uint16_t Xpos, uint16_t Ypos, uint16_t orientation){
	if (orientation == 1){
		for (int i = Xpos; i < (Xpos+3); i++){
			LCD_Erase_Square(i,Ypos);
			LCD_Screen[Ypos][i] = 0;
			Obj_Color[Ypos][i] = GameScreenColor;
		}
		LCD_Erase_Square(Xpos+1,Ypos-1);
		LCD_Screen[Ypos-1][Xpos+1] = 0;
		Obj_Color[Ypos-1][Xpos+1] = GameScreenColor;
	}

	if (orientation == 3){
		for (int i = Xpos; i < (Xpos+3); i++){
			LCD_Erase_Square(i,Ypos);
			LCD_Screen[Ypos][i] = 0;
			Obj_Color[Ypos][i] = GameScreenColor;
		}
		LCD_Erase_Square(Xpos+1,Ypos+1);
		LCD_Screen[Ypos+1][Xpos+1] = 0;
		Obj_Color[Ypos+1][Xpos+1] = GameScreenColor;
	}
	if (orientation == 2){
		for (int i = Ypos-1; i < (Ypos+2); i++){
			LCD_Erase_Square(Xpos,i);
			LCD_Screen[i][Xpos] = 0;
			Obj_Color[i][Xpos] = GameScreenColor;
		}
		LCD_Erase_Square(Xpos+1,Ypos);
		LCD_Screen[Ypos][Xpos+1] = 0;
		Obj_Color[Ypos][Xpos+1] = GameScreenColor;
	}
	if (orientation == 4){
		for (int i = Ypos-1; i < (Ypos+2); i++){
			LCD_Erase_Square(Xpos+1,i);
			LCD_Screen[i][Xpos+1] = 0;
			Obj_Color[i][Xpos+1] = GameScreenColor;
		}
		LCD_Erase_Square(Xpos,Ypos);
		LCD_Screen[Ypos][Xpos] = 0;
		Obj_Color[Ypos][Xpos] = GameScreenColor;
	}
}

// Draw Block Seven at X, Y, and in given orientation
void drawBlockSeven(uint16_t Xpos, uint16_t Ypos, uint16_t orientation){
	if (orientation == 1 || orientation == 3){
		for (int i = Xpos; i < (Xpos+2); i++){
			LCD_Draw_Square_Fill_Border(i,Ypos-1,LCD_COLOR_RED);
			LCD_Screen[Ypos-1][i] = 1;
			Obj_Color[Ypos-1][i] = LCD_COLOR_RED;
			LCD_Draw_Square_Fill_Border(i+1,Ypos,LCD_COLOR_RED);
			LCD_Screen[Ypos][i+1] = 1;
			Obj_Color[Ypos][i+1] = LCD_COLOR_RED;
		}
	}
	// Y-1		1	1
	//   Y	    	1	1
	// Y+1
	// Y+2
	//	   X-1  X  X+1  X+2
	if (orientation == 2 || orientation == 4){
		for (int i = Ypos; i < (Ypos+2); i++){
			LCD_Draw_Square_Fill_Border(Xpos,i,LCD_COLOR_RED);
			LCD_Screen[i][Xpos] = 1;
			Obj_Color[i][Xpos] = LCD_COLOR_RED;
			LCD_Draw_Square_Fill_Border(Xpos+1,i-1,LCD_COLOR_RED);
			LCD_Screen[i-1][Xpos+1] = 1;
			Obj_Color[i-1][Xpos+1] = LCD_COLOR_RED;
		}
// Y-1			1
//   Y	    1	1
// Y+1		1
// Y+2
//	   X-1  X  X+1  X+2
	}
}

// Erase Block Seven at X, Y, and in given orientation
void eraseBlockSeven(uint16_t Xpos, uint16_t Ypos, uint16_t orientation){
	if (orientation == 1 || orientation == 3){
		for (int i = Xpos; i < (Xpos+2); i++){
			LCD_Erase_Square(i,Ypos-1);
			LCD_Screen[Ypos-1][i] = 0;
			Obj_Color[Ypos-1][i] = GameScreenColor;
			LCD_Erase_Square(i+1,Ypos);
			LCD_Screen[Ypos][i+1] = 0;
			Obj_Color[Ypos][i+1] = GameScreenColor;
		}
	}
	if (orientation == 2 || orientation == 4){
		for (int i = Ypos; i < (Ypos+2); i++){
			LCD_Erase_Square(Xpos,i);
			LCD_Screen[i][Xpos] = 0;
			Obj_Color[i][Xpos] = GameScreenColor;
			LCD_Erase_Square(Xpos+1,i-1);
			LCD_Screen[i-1][Xpos+1] = 0;
			Obj_Color[i-1][Xpos+1] = GameScreenColor;
		}
	}
}

// Draw a black bottom border at bottom of screen
void drawBottomBorder(){
	for (uint16_t i = 0; i < 12; i++){
		LCD_Draw_Square_Fill_Border(i, 15, LCD_COLOR_BLACK);
		LCD_Screen[15][i] = 1;
		Obj_Color[15][i] = LCD_COLOR_BLACK;
	}
	HAL_Delay(1000);
}

// Erase block in current orientation and redraw in new orientation
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

// Function to determine if blocks are able to rotate or if there is a block already there preventing them
int ableToRotate(){
//	uint16_t BlockNum = CurrentBlock[0];
//	uint16_t Xpos = CurrentBlock[1];
//	uint16_t Ypos = CurrentBlock[2];
	uint16_t OrgOrientation = CurrentBlock[3];
//	uint16_t NewOrientation;
	if (OrgOrientation < 4){
		CurrentBlock[3] = OrgOrientation + 1;
	}
	else if (OrgOrientation == 4){
		CurrentBlock[3] = 1;
	}
	int full = isFull();
	CurrentBlock[3] = OrgOrientation;
	return full;
}

// Draw the block based on values in CurrentBlock array
void drawCurrentBlock(){
	uint16_t BlockNum = CurrentBlock[0];
	uint16_t Xpos = CurrentBlock[1];
	uint16_t Ypos = CurrentBlock[2];
	uint16_t OrgOrientation = CurrentBlock[3];

	if (BlockNum == 1){
		drawBlockOne(Xpos, Ypos, OrgOrientation);
	}
	if (BlockNum == 2){
		drawBlockTwo(Xpos, Ypos, OrgOrientation);
	}
	if (BlockNum == 3){
		drawBlockThree(Xpos, Ypos, OrgOrientation);
	}
	if (BlockNum == 4){
		drawBlockFour(Xpos, Ypos, OrgOrientation);
	}
	if (BlockNum == 5){
		drawBlockFive(Xpos, Ypos, OrgOrientation);
	}
	if (BlockNum == 6){
		drawBlockSix(Xpos, Ypos, OrgOrientation);
	}
	if (BlockNum == 7){
		drawBlockSeven(Xpos, Ypos, OrgOrientation);
	}
}

// Erase the block based on values in CurrentBlock array
void eraseCurrentBlock(){
	uint16_t BlockNum = CurrentBlock[0];
	uint16_t Xpos = CurrentBlock[1];
	uint16_t Ypos = CurrentBlock[2];
	uint16_t OrgOrientation = CurrentBlock[3];

	if (BlockNum == 1){
		eraseBlockOne(Xpos, Ypos, OrgOrientation);
	}
	if (BlockNum == 2){
		eraseBlockTwo(Xpos, Ypos, OrgOrientation);
	}
	if (BlockNum == 3){
		eraseBlockThree(Xpos, Ypos, OrgOrientation);
	}
	if (BlockNum == 4){
		eraseBlockFour(Xpos, Ypos, OrgOrientation);
	}
	if (BlockNum == 5){
		eraseBlockFive(Xpos, Ypos, OrgOrientation);
	}
	if (BlockNum == 6){
		eraseBlockSix(Xpos, Ypos, OrgOrientation);
	}
	if (BlockNum == 7){
		eraseBlockSeven(Xpos, Ypos, OrgOrientation);
	}
}

// Update the values in CurrentBlock array
void updateCurrentBlock(uint16_t BlockNum, uint16_t Xpos, uint16_t Ypos, uint16_t Orientation){
	CurrentBlock[0] = BlockNum;
	CurrentBlock[1] = Xpos;
	CurrentBlock[2] = Ypos;
	CurrentBlock[3] = Orientation;
}

// Return current Y position of current block
uint16_t getCurrentYpos(){
	return CurrentBlock[2];
}

// Update the Y position of current block
uint16_t updateYpos(){
	CurrentBlock[2] += 1;
	return CurrentBlock[2];
}

// Update the X position of current block
void updateXpos(int dir){
	if (dir == 1){
		CurrentBlock[1] -= 1;
	}
	if (dir == 2){
		CurrentBlock[1] += 1;
	}
}

// Function to see if space to left of block is full
int isLeftFull(){
	uint16_t BlockNum = CurrentBlock[0];
	uint16_t Xpos = CurrentBlock[1];
	uint16_t Ypos = CurrentBlock[2];
	uint16_t Orientation = CurrentBlock[3];

	if (Xpos-1 >= 0){
		if (BlockNum == 1){
			if (Orientation == 1 || Orientation == 3){
				if (LCD_Screen[Ypos][Xpos-1] == 1){
					return 0;
				}
			}
			if (Orientation == 2 || Orientation == 4){
				for (int i = Ypos-1; i < Ypos+3; i++){
					if (LCD_Screen[i][Xpos-1] == 1){
						return 0;
					}
				}
			}
		}

	if (BlockNum == 2){
		if (Orientation == 1){
			for (int i = Ypos-1; i < Ypos+1; i++){
				if (LCD_Screen[i][Xpos-1] == 1){
					return 0;
				}
			}
		}
		if (Orientation == 2){
			for (int i = Ypos-1; i < Ypos+2; i++){
				if (LCD_Screen[i][Xpos-1] == 1){
					return 0;
				}
			}
		}
		if (Orientation == 3){
			if (LCD_Screen[Ypos][Xpos-1] == 1 || LCD_Screen[Ypos+1][Xpos+1] == 1){
					return 0;
				}
			}
		if (Orientation == 4){
			for (int i = Ypos-1; i < Ypos+1; i++){
				if (LCD_Screen[i][Xpos] == 1){
					return 0;
				}
			}
			if (LCD_Screen[Ypos+1][Xpos-1] == 1){
				return 0;
			}
		}
	}

	if (BlockNum == 3){
		if (Orientation == 1){
			if (LCD_Screen[Ypos][Xpos-1] == 1){
				return 0;
			}
			if (LCD_Screen[Ypos-1][Xpos+1] == 1){
				return 0;
			}
		}
		if (Orientation == 2){
			for (int i = Ypos-1; i < Ypos+1; i++){
				if (LCD_Screen[i][Xpos-1] == 1){
					return 0;
				}
			}
		}
		if (Orientation == 3){
			for (int i = Ypos; i < Ypos+2; i++){
				if (LCD_Screen[i][Xpos-1] == 1){
					return 0;
				}
			}
		}
		if (Orientation == 4){
			for (int i = Ypos; i < Ypos+2; i++){
				if (LCD_Screen[i][Xpos] == 1){
					return 0;
				}
			}
			if (LCD_Screen[Ypos-1][Xpos-1] == 1){
				return 0;
			}
		}
	}

	if (BlockNum == 4){
		if (LCD_Screen[Ypos][Xpos-1] == 1){
			return 0;
		}
		if (LCD_Screen[Ypos-1][Xpos-1] == 1){
			return 0;
		}
	}

	if (BlockNum == 5){
		if (Orientation == 1 || Orientation == 3){
			if (LCD_Screen[Ypos][Xpos-1] == 1){
				return 0;
			}
			if (LCD_Screen[Ypos-1][Xpos] == 1){
				return 0;
			}
		}
		if (Orientation == 2 || Orientation == 4){
			for (int i = Ypos-1; i < Ypos+1; i++){
				if (LCD_Screen[i][Xpos-1] == 1){
					return 0;
				}
			}
			if (LCD_Screen[Ypos+1][Xpos] == 1){
				return 0;
			}
		}
	}

	if (BlockNum == 6){
		if (Orientation == 1){
			if (LCD_Screen[Ypos][Xpos-1] == 1){
				return 0;
			}
			if (LCD_Screen[Ypos-1][Xpos] == 1){
				return 0;
			}
		}
		if (Orientation == 2){
			for (int i = Ypos-1; i < (Ypos+2); i++){
				if (LCD_Screen[i][Xpos-1] == 1){
					return 0;
				}
			}
		}
		if (Orientation == 3){
			if (LCD_Screen[Ypos][Xpos-1] == 1){
				return 0;
			}
			if (LCD_Screen[Ypos+1][Xpos] == 1){
				return 0;
			}
		}
		if (Orientation == 4){
			if (LCD_Screen[Ypos-1][Xpos] == 1){
				return 0;
			}
			if (LCD_Screen[Ypos][Xpos-1] == 1){
				return 0;
			}
			if (LCD_Screen[Ypos+1][Xpos] == 1){
				return 0;
			}
		}
		}

	if (BlockNum == 7){
		if (Orientation == 1 || Orientation == 3){
			if (LCD_Screen[Ypos-1][Xpos-1] == 1){
				return 0;
			}
			if (LCD_Screen[Ypos][Xpos] == 1){
				return 0;
			}
		}
		if (Orientation == 2 || Orientation == 4){
			for (int i = Ypos; i < Ypos+2; i++){
				if (LCD_Screen[i][Xpos-1] == 1){
					return 0;
				}
			}
			if (LCD_Screen[Ypos-1][Xpos] == 1){
				return 0;
			}
	}
	}
	}
	return 1;
}

// Function to determine if block can move left if left is not full & Xpos is greater than 0
int canMoveLeft(){
//	uint16_t BlockNum = CurrentBlock[0];
	uint16_t Xpos = CurrentBlock[1];
//	uint16_t Ypos = CurrentBlock[2];
//	uint16_t Orientation = CurrentBlock[3];

//	if (BlockNum == 1){
//		if (Orientation == 1 || Orientation == 3){
//			if (Xpos > 0){
//					return 1;
//			}
//		}
//		if (Orientation == 2 || Orientation == 4){
//			if (Xpos > 0){
//					return 1;
//			}
//		}
//		return 0;
//	}
//	if (BlockNum == 2 || BlockNum == 3){
//		if (Xpos > 0){
//			return 1;
//		}
//	}
//	if (BlockNum == 4 || BlockNum == 6){
//		if (Xpos > 0){
//			return 1;
//		}
//	}
//	if (BlockNum == 5 || BlockNum == 7){
//		if (Xpos > 0){
//			return 1;
//		}
//	}

	if (Xpos > 0){
		if (isLeftFull() == 1){
			return 1;
		}
		else return 0;
	}
	return 0;
}

// Function to see if space to right of block is full
int isRightFull(){
	uint16_t BlockNum = CurrentBlock[0];
	uint16_t Xpos = CurrentBlock[1];
	uint16_t Ypos = CurrentBlock[2];
	uint16_t Orientation = CurrentBlock[3];

//	if (Xpos+1 <= 11){
		if (BlockNum == 1){
			if (Orientation == 1 || Orientation == 3){
				if (LCD_Screen[Ypos][Xpos+4] == 1){
					return 0;
				}
			}
			if (Orientation == 2 || Orientation == 4){
				for (int i = Ypos-1; i<Ypos+3; i++){
					if (LCD_Screen[i][Xpos+1] == 1){
						return 0;
					}
				}
			}
		}

		if (BlockNum == 2){
			if (Orientation == 1){
				if (LCD_Screen[Ypos-1][Xpos+1] == 1){
					return 0;
				}
				if (LCD_Screen[Ypos][Xpos+3] == 1){
					return 0;
				}
			}
			if (Orientation == 2){
				for (int i = Ypos; i < Ypos+2; i++){
					if (LCD_Screen[i][Xpos+1] == 1){
						return 0;
					}
				}
				if (LCD_Screen[Ypos-1][Xpos+2] == 1){
					return 0;
				}
			}
			if (Orientation == 3){
				if (LCD_Screen[Ypos][Xpos+3] == 1){
					return 0;
				}
				if (LCD_Screen[Ypos+1][Xpos+3] == 1){
					return 0;
				}
			}
			if (Orientation == 4){
				for (int i = Ypos-1; i < (Ypos+2); i++){
					if (LCD_Screen[i][Xpos+2] == 1){
						return 0;
					}
				}
			}
		}

		if (BlockNum == 3){
			if (Orientation == 1){
				if (LCD_Screen[Ypos][Xpos+3] == 1){
					return 0;
				}
				if (LCD_Screen[Ypos-1][Xpos+3] == 1){
					return 0;
				}
			}
			if (Orientation == 2){
				for (int i = Ypos-1; i < Ypos+1; i++){
					if (LCD_Screen[i][Xpos+1] == 1){
						return 0;
					}
				}
				if (LCD_Screen[Ypos+1][Xpos+2] == 1){
					return 0;
				}
			}
			if (Orientation == 3){
				if (LCD_Screen[Ypos][Xpos+3] == 1){
					return 0;
				}
				if (LCD_Screen[Ypos+1][Xpos+1] == 1){
					return 0;
				}
			}
			if (Orientation == 4){
				for (int i = Ypos-1; i < (Ypos+2); i++){
					if (LCD_Screen[i][Xpos+2] == 1){
						return 0;
					}
				}
			}
		}

		if (BlockNum == 4){
			if (LCD_Screen[Ypos][Xpos+2] == 1){
				return 0;
			}
			if (LCD_Screen[Ypos-1][Xpos+2] == 1){
				return 0;
			}
		}

		if (BlockNum == 5){
			if (Orientation == 1 || Orientation == 3){
				if (LCD_Screen[Ypos-1][Xpos+3] == 1){
					return 0;
				}
				if (LCD_Screen[Ypos][Xpos+2] == 1){
					return 0;
				}
			}
			if (Orientation == 2 || Orientation == 4){
				for (int i = Ypos; i < Ypos+2; i++){
					if (LCD_Screen[i][Xpos+2] == 1){
						return 0;
					}
				}
				if (LCD_Screen[Ypos-1][Xpos+1] == 1){
					return 0;
				}
			}
		}

		if (BlockNum == 6){
			if (Orientation == 1){
				if (LCD_Screen[Ypos][Xpos+3] == 1 || LCD_Screen[Ypos-1][Xpos+2] == 1){
					return 0;
				}
			}
			if (Orientation == 2){
				if (LCD_Screen[Ypos-1][Xpos+2] == 1 || LCD_Screen[Ypos][Xpos+3] == 1 || LCD_Screen[Ypos+1][Xpos+2] == 1){
					return 0;
				}
			}
			if (Orientation == 3){
				if (LCD_Screen[Ypos][Xpos+3] == 1 || LCD_Screen[Ypos+1][Xpos+2] == 1){
					return 0;
				}
			}
			if (Orientation == 4){
				for (int i = Ypos-1; i < Ypos+2; i++){
					if (LCD_Screen[i][Xpos+2] == 1){
						return 0;
					}
				}
			}
		}

		if (BlockNum == 7){
			if (Orientation == 1 || Orientation == 3){
				if (LCD_Screen[Ypos-1][Xpos+2] == 1 || LCD_Screen[Ypos][Xpos+3] == 1){
					return 0;
				}
			}
			if (Orientation == 2 || Orientation == 4){
				for (int i = Ypos-1; i < Ypos+1; i++){
					if (LCD_Screen[i][Xpos+2] == 1){
						return 0;
					}
				}
				if (LCD_Screen[Ypos+1][Xpos+1] == 1){
					return 0;
				}
			}
		}
		return 1;
//	}
}

// Function to determine if block can move right if right is not full & Xpos is small enough depending on block
int canMoveRight(){
	uint16_t BlockNum = CurrentBlock[0];
	uint16_t Xpos = CurrentBlock[1];
//	uint16_t Ypos = CurrentBlock[2];
	uint16_t Orientation = CurrentBlock[3];

	if (isRightFull() == 1){
		if (BlockNum == 1){
			if (Orientation == 1 || Orientation == 3){
				if (Xpos < 8){
						return 1;
				}
			}
			if (Orientation == 2 || Orientation == 4){
				if (Xpos < 11){
						return 1;
				}
			}
		}
		if (BlockNum == 2 || BlockNum == 3){
			if (Orientation == 1 || Orientation == 3){
				if (Xpos < 9){
					return 1;
				}
			}
			if (Orientation == 2 || Orientation == 4){
				if (Xpos < 10){
					return 1;
				}
			}
		}
		if (BlockNum == 4){
			if (Xpos < 10){
				return 1;
			}
		}
	//	if (BlockNum == 5){
	//		if (Xpos < 10){
	//			return 1;
	//		}
	//	}
		if (BlockNum == 5 || BlockNum == 6 || BlockNum == 7){
			if (Orientation == 1 || Orientation == 3){
				if (Xpos < 9){
					return 1;
				}
			}
			if (Orientation == 2 || Orientation == 4){
				if (Xpos < 10){
					return 1;
				}
			}
		}

	}
	return 0;
}

// Function to check if space below block is full or if it can move down
int isFull(){
	uint16_t BlockNum = CurrentBlock[0];
	uint16_t Xpos = CurrentBlock[1];
	uint16_t Ypos = CurrentBlock[2];
	uint16_t Orientation = CurrentBlock[3];

	if (BlockNum == 1){
		if (Orientation == 1 || Orientation == 3){
			for (uint16_t i = Xpos; i < Xpos+4; i++){
				if (LCD_Screen[Ypos+1][i] == 1){
					return 0;
				}
			}
		}
		if (Orientation == 2 || Orientation == 4){
			if (LCD_Screen[Ypos+3][Xpos] == 1){
				return 0;
			}
		}
		return 1;
	}

	if (BlockNum == 2){
		if (Orientation == 1){
			for(uint16_t i = Xpos; i < Xpos+3; i++){
				if (LCD_Screen[Ypos+1][i] == 1)
				{
					return 0;
				}
			}
		}
		if (Orientation == 2){
			if (LCD_Screen[Ypos+2][Xpos] == 1 || LCD_Screen[Ypos][Xpos+1] == 1){
				return 0;
			}
		}
		if (Orientation == 3){
			if (LCD_Screen[Ypos+1][Xpos] == 1 || LCD_Screen[Ypos+1][Xpos+1] == 1 || LCD_Screen[Ypos+2][Xpos+2] == 1) {
				return 0;
			}
		}
		if (Orientation == 4){
			if (LCD_Screen[Ypos+2][Xpos] == 1 || LCD_Screen[Ypos+2][Xpos+1] == 1){
				return 0;
			}
		}
		return 1;
	}

	if (BlockNum == 3){
		if (Orientation == 1){
			for(uint16_t i = Xpos; i < Xpos+3; i++){
				if (LCD_Screen[Ypos+1][i] == 1)
				{
					return 0;
				}
			}
		}
		if (Orientation == 2){
			if (LCD_Screen[Ypos+2][Xpos] == 1 || LCD_Screen[Ypos+2][Xpos+1] == 1){
				return 0;
			}
		}
		if (Orientation == 3){
			if (LCD_Screen[Ypos+2][Xpos] == 1 || LCD_Screen[Ypos+1][Xpos+1] == 1 || LCD_Screen[Ypos+1][Xpos+2] == 1) {
				return 0;
			}
		}
		if (Orientation == 4){
			if (LCD_Screen[Ypos][Xpos] == 1 || LCD_Screen[Ypos+2][Xpos+1] == 1) {
				return 0;
			}
		}
		return 1;
	}

	if (BlockNum == 4){
		if (LCD_Screen[Ypos+1][Xpos] == 1 || LCD_Screen[Ypos+1][Xpos+1] == 1) {
			return 0;
		}
		return 1;
	}

	if (BlockNum == 5){
		if (Orientation == 1 || Orientation == 3){
			if (LCD_Screen[Ypos+1][Xpos] == 1 || LCD_Screen[Ypos+1][Xpos+1] == 1 || LCD_Screen[Ypos][Xpos+2] == 1) {
				return 0;
			}
		}
		if (Orientation == 2 || Orientation == 4){
			if (LCD_Screen[Ypos+1][Xpos] == 1 || LCD_Screen[Ypos+2][Xpos+1] == 1) {
				return 0;
			}
		}
		return 1;
	}

	if (BlockNum == 6){
		if (Orientation == 1){
			for (uint16_t i = Xpos; i < Xpos+3; i++){
				if (LCD_Screen[Ypos+1][i] == 1){
					return 0;
				}
			}
		}
		if (Orientation == 2){
			if (LCD_Screen[Ypos+2][Xpos] == 1 || LCD_Screen[Ypos+1][Xpos+1] == 1){
				return 0;
			}
		}
		if (Orientation == 3){
			if (LCD_Screen[Ypos+1][Xpos] == 1 || LCD_Screen[Ypos+2][Xpos+1] == 1 || LCD_Screen[Ypos+1][Xpos+2] == 1){
				return 0;
			}
		}
		if (Orientation == 4){
			if (LCD_Screen[Ypos+1][Xpos] == 1 || LCD_Screen[Ypos+2][Xpos+1] == 1){
				return 0;
			}
		}
		return 1;
	}

	if (BlockNum == 7){
		if (Orientation == 1 || Orientation == 3){
			if (LCD_Screen[Ypos][Xpos] == 1 || LCD_Screen[Ypos+1][Xpos+1] == 1 || LCD_Screen[Ypos+1][Xpos+2] == 1) {
				return 0;
			}
		}
		if (Orientation == 2 || Orientation == 4){
			if (LCD_Screen[Ypos+2][Xpos] == 1 || LCD_Screen[Ypos+1][Xpos+1] == 1) {
				return 0;
			}
		}
		return 1;
	}
	return 1;
}

// Function to increase a counter for seconds
void addSecond(){
	gameTime += 1;
}

// Function to update the highest_row array
int updateTop(){
//	uint16_t BlockNum = CurrentBlock[0];
//	uint16_t Xpos = CurrentBlock[1];
	uint16_t Ypos = CurrentBlock[2];
//	uint16_t Orientation = CurrentBlock[3];
	int Highest_Row = 14;

	for (int i = Ypos-2; i < 15; i++){
		for (int j = 0; j < 12; j++){
			if (LCD_Screen[i][j] == 1){
				if (i < topRow[j]){
					topRow[j] = i;
					if (i < Highest_Row){
						Highest_Row = i;
					}
				}
			}
		}
	}
	return Highest_Row;
}

// Function to check if a row is complete, and if so remove it and count how many lines have been moved
void checkForTetris(int Highest_Row){
	int rowComplete = 0;
	int consecutive = 0;
	int count = 0;
	int completeRows[4] = {0};
	int arrCount = 0;

//	int Highest_Row = 14;
//		for (int i = 0; i < 12; i++){
//			if (topRow[i] < Highest_Row){
//				Highest_Row = topRow[i];
//			}
//		}

	for (int i = 14; i >= (Highest_Row-1); i--){
		for (int j = 0; j < 12; j++){
			// Check if all elements are full
			if (LCD_Screen[i][j] == 1){
				 rowComplete = 1;
			}
			else if (LCD_Screen[i][j] != 1){
				rowComplete = 0;
				break;
			}
		}

		// if there is a full row, add one to counter and check to see if consecutive
		if (rowComplete == 1)
		{
			completeRows[arrCount] = i;
			arrCount += 1;
			if (consecutive == 1)
			{
//				shiftRowDown(rowNum);
				count += 1;
			}
			else if (consecutive == 0){
//				shiftRowDown(rowNum);
				count += 1;
				consecutive = 1;
			}
		}
		// Once you have an incomplete row, check the value of count and this tells you how many lines were cleared in a row
		else if(rowComplete == 0){
			if (consecutive == 1){
				if (count == 1){
					result[1] += 1;
				}
				if (count == 2){
					result[2] += 1;
				}
				if (count == 3){
					result[3] += 1;
				}
				if (count == 4){
					result[4] += 1;
				}
				consecutive = 0;
				count = 0;
			}
		}
}
	// Erase and shift down the rows that were found to have been complete
	int shiftedCount = 0;
	for (int k = 0; k < arrCount; k++){
		shiftRowDown((completeRows[k]+shiftedCount));
		shiftedCount += 1;
	}

}

// Once a row is complete, we erase that row and shift the remaining rows down
void shiftRowDown(int rowNum){
	int Highest_Row = 14;
	for (int i = 0; i < 12; i++){
		if (topRow[i] < Highest_Row){
			Highest_Row = topRow[i];
		}
	}

	for (int j = rowNum; j >= Highest_Row; j--){
		for (int k = 0; k < 12; k++){
			uint16_t Above_Value = Obj_Color[j-1][k];
			if (Above_Value == LCD_COLOR_BLACK){
				Above_Value = GameScreenColor;
			}
			LCD_Screen[j][k] = LCD_Screen[j-1][k];
			Obj_Color[j][k] = Above_Value;
			LCD_Draw_Square_Fill_Border(k,j,Above_Value);
		}
	}

//	for (int l = 0; l < 12; l++){
//		LCD_Screen[Highest_Row][l] = 0;
//		Obj_Color[Highest_Row][l] = 0;
//		LCD_Erase_Square(l,Highest_Row);
//	}
}

// Function to change block Ypos and move it down
void moveBlockDown(){
	  eraseCurrentBlock();
	  updateYpos();
	  drawCurrentBlock();
}

// Function to change block Xpos and move it left
void moveBlockLeft(){
	eraseCurrentBlock();
	updateXpos(1);
	drawCurrentBlock();
}

// Function to change block Xpos and move it right
void moveBlockRight(){
	eraseCurrentBlock();
	updateXpos(2);
	drawCurrentBlock();
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
