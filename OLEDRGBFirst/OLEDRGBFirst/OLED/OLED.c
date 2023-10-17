/*
 * OLED.c
 *
 * Created: 2022-04-13 18:57:17
 *  Author: tmf
 */ 

#include <OLED\OLED.h>
#include <SPI.h>
#include <util\delay.h>

uint8_t OLED_X, OLED_Y;
uint16_t OLED_FgColor, OLED_BgColor;	//Kolor t³a i pierwszego planu

void OLED_Init()
{
	OLED_RESET();   //Zresetuj kontroler

  //  OLED_Send_CmdWithByte(SSD1351_CMD_COMMANDLOCK, 0x12);
  //  OLED_Send_CmdWithByte(SSD1351_CMD_COMMANDLOCK, 0xB1);
    OLED_Send_Cmd(SSD1351_CMD_DISPLAYOFF); // Display off, no args
    OLED_Send_CmdWithByte(SSD1351_CMD_CLOCKDIV, 0xF1);  // 7:4 = Oscillator Freq, 3:0 = CLK Div Ratio (A[3:0]+1 = 1..16)
    OLED_Send_CmdWithByte(SSD1351_CMD_MUXRATIO, 127);
    OLED_Send_CmdWithByte(SSD1351_CMD_DISPLAYOFFSET, 0x5f);		//96 linia - niektóre OLEDY maj¹ przesuniêcie
    OLED_Send_CmdWithByte(SSD1351_CMD_STARTLINE, 0x20);
	OLED_Send_CmdWithByte(SSD1351_CMD_SETGPIO, 0x00);
//	OLED_Send_CmdWithByte(SSD1351_CMD_SETREMAP, 0b10100100);		//Bity 7-6 - tryb 262k kolorów,bit 3 == 1 - RGB zamiast BGR
	OLED_Send_CmdWithByte(SSD1351_CMD_SETREMAP, 0b01100100);		//Bity 7-6 - tryb 262k kolorów,bit 3 == 1 - RGB zamiast BGR
    OLED_Send_CmdWithByte(SSD1351_CMD_FUNCTIONSELECT, 0x01); // internal (diode drop)
    OLED_Send_CmdWithByte(SSD1351_CMD_PRECHARGE, 0x32);
    OLED_Send_CmdWithByte(SSD1351_CMD_VCOMH, 0x05);
    OLED_Send_Cmd(SSD1351_CMD_NORMALDISPLAY);
    OLED_Send_CmdWithBytes(SSD1351_CMD_CONTRASTABC, 3, (uint8_t[]) {0xC8, 0x80, 0xC0});
    OLED_Send_CmdWithByte(SSD1351_CMD_CONTRASTMASTER, 0x0F);
    OLED_Send_CmdWithBytes(SSD1351_CMD_SETVSL, 3, (uint8_t[]) {0xA0, 0xB5, 0x55});
    OLED_Send_CmdWithByte(SSD1351_CMD_PRECHARGE2, 0x01);
	OLED_Send_CmdWithBytes(SSD1351_CMD_DISPLAYENHANCE, 3, (uint8_t[]) {0xA4, 0x00, 0x00});	//??
    
	_delay_ms(100);
	OLED_Send_Cmd(SSD1351_CMD_DISPLAYON);
	_delay_ms(300);
}

void OLED_SetWindow(uint8_t xs, uint8_t ys, uint8_t xe, uint8_t ye)
{
	OLED_Send_CmdWithBytes(SSD1351_CMD_SETCOLUMN, 2, (uint8_t[]) {xs, xe});
	OLED_Send_CmdWithBytes(SSD1351_CMD_SETROW, 2, (uint8_t[]) {ys, ye});
}

void OLED_Clear(uint16_t color)
{
	OLED_SetWindow(0, 0, OLED_WIDTH - 1, OLED_HEIGHT - 1);	//Okno zapisu
	OLED_Send_Cmd(SSD1351_CMD_WRITERAM);					//Zapis do VRAM
	OLED_CS(0);
	uint16_t pixno = OLED_HEIGHT * OLED_WIDTH;
/*	for(uint8_t y = 0; y < OLED_HEIGHT; y++)
	{
		for(uint16_t x = 0; x < OLED_WIDTH; x++)
		{
			SPI_Write_Byte(color >> 8);
			SPI_Write_Byte(color & 0xff);
//			ssd2119_SendDataByte(color >> 8);
//			ssd2119_SendDataByte(color & 0xff);
		}
	}*/
	while(pixno--)
	{
		OLED_SendDataByte(color >> 8);
		OLED_SendDataByte(color & 0xff);
		//SPI_Write_Byte(color >> 8);	
		//SPI_Write_Byte(color & 0xff);
	}
	OLED_SendDataByte(color >> 8);
	OLED_ClearTXCIF();
	OLED_SendDataByte(color & 0xff);
	OLED_WaitForSent();
	OLED_CS(1);
}

void OLED_SetPixel(uint8_t x, uint8_t y, uint16_t color)
{
	OLED_SetWindow(x, y, OLED_WIDTH - 1, OLED_HEIGHT - 1);	//Okno zapisu
	OLED_Send_Cmd(SSD1351_CMD_WRITERAM);					//Zapis do VRAM
	OLED_CS(0);
	OLED_SendDataByte(color >> 8);
	SPI_Write_Byte(color & 0xff);
	OLED_CS(1);
}

void OLED_SetPixel18(uint8_t x, uint8_t y, uint32_t color)
{
	OLED_SetWindow(x, y, OLED_WIDTH - 1, OLED_HEIGHT - 1);	//Okno zapisu
	OLED_Send_Cmd(SSD1351_CMD_WRITERAM);					//Zapis do VRAM
	OLED_CS(0);
	OLED_SendDataByte(color & 0xff);
	OLED_SendDataByte((color >> 8) & 0xff);
	SPI_Write_Byte((color >> 16) & 0xff);
	OLED_CS(1);
}

void OLED_SetText(uint8_t x, uint8_t y, const char *tekst, const uint8_t __flash * const __flash font[])
{
	uint8_t rows=(uint8_t)(uint16_t)font[0]; //Pobierz wysokoœæ fontu
	y+=rows-1;
	char ch;

	while((ch=*tekst++))  //Wyœwietl kolejne znaki a¿ do koñca tekstu (znaku NUL)
	{
		const uint8_t __flash *znak=font[ch-30]; //Adres pocz¹tku opisu znaku
		uint8_t col=*znak++;                     //Szerokoœæ znaku w pikselach
		uint8_t page=0, coldesc=0, colmask=0;

		for(uint8_t ox=0; ox < col; ox++)        //Wyœwietlamy kolejne kolumny tworz¹ce znak
		{
			uint8_t dispmask=1 << (y % 8);
			for(uint8_t oy=0; oy < rows; oy++)   //Narysuj jedn¹ kolumnê znaku
			{
				if(colmask == 0)
				{
					colmask=0x80;
					coldesc=*znak++;             //Pobierz bajt opisu znaku
				}
				page=((y - oy) >> 3) & 0b111;    //Zabezpieczenie przed zapisem poza bufor
//				OLED_FrameBuffer[page][x]&=~dispmask;
//				if(coldesc & colmask) OLED_FrameBuffer[page][x]|=dispmask;
				colmask>>=1;
				dispmask>>=1;
				if(dispmask == 0)   //Przekraczamy stronê - nale¿y zapisaæ kompletny bajt
				{
//					OLED_FrameBuffer[page][OLED_WIDTH]=BufferDirty;
					dispmask=0x80;
				}
			}
//			if(dispmask != 0x80) OLED_FrameBuffer[page][OLED_WIDTH]=BufferDirty;
			x++;
			if(x == OLED_WIDTH) return; //Wychodzimy za LCD
		}
	}
}

void OLED_SetTextRotate(uint8_t x, uint8_t y, const char *tekst, const uint8_t __flash * const __flash font[], _Bool invert)
{
	uint8_t rows=(uint8_t)(uint16_t)font[0]; //Pobierz wysokoœæ fontu
	//y+=rows-1;
	char ch;

	while((ch=*tekst++))  //Wyœwietl kolejne znaki a¿ do koñca tekstu (znaku NUL)
	{
		const uint8_t __flash *znak=font[ch-30]; //Adres pocz¹tku opisu znaku
		uint8_t col=*znak++;                     //Szerokoœæ znaku w pikselach
		uint8_t coldesc=0, colmask=0;

		for(uint8_t ox=0; ox < col; ox++)        //Wyœwietlamy kolejne kolumny tworz¹ce znak
		{
			//for(uint8_t oy=0; oy < rows; oy++)   //Narysuj jedn¹ kolumnê znaku
			for(int8_t oy = rows - 1; oy >= 0; oy--) 
			{
				if(colmask == 0)
				{
					colmask=0x80;
					coldesc=*znak++;             //Pobierz bajt opisu znaku
				}
				if(coldesc & colmask)
					OLED_SetPixel(y + oy, x, invert);	//Ró¿ne kombinacje x,y umo¿liwiaj¹ dowolne obroty znaku
				colmask>>=1;
			}
			x++;
			if(x == OLED_WIDTH) return; //Wychodzimy za LCD
		}
	}
}

void OLED_SetTextOp(uint8_t x, uint8_t y, const char *tekst, const uint8_t __flash * const __flash font[], _Bool invert)
{
	uint8_t rows=(uint8_t)(uint16_t)font[0]; //Pobierz wysokoœæ fontu
	char ch;
	uint16_t tmpBgClr, tmpFgClr;
	tmpBgClr = OLED_BgColor;
	tmpFgClr = OLED_FgColor;
	if(invert)					//Ew. inwersja kolorów
	{
		tmpBgClr = OLED_FgColor;
		tmpFgClr = OLED_BgColor;
	}

	while((ch=*tekst++))  //Wyœwietl kolejne znaki a¿ do koñca tekstu (znaku NUL)
	{
		const uint8_t __flash *znak=font[ch-30]; //Adres pocz¹tku opisu znaku
		uint8_t col=*znak++;                     //Szerokoœæ znaku w pikselach
		uint8_t coldesc=0, colmask=0;

		for(uint8_t ox = 0; ox < col; ox++)        //Wyœwietlamy kolejne kolumny tworz¹ce znak
		{
			for(int8_t oy = rows - 1; oy >= 0; oy--) //Narysuj jedn¹ kolumnê znaku
			{
				if(colmask == 0)
				{
					colmask=0x80;
					coldesc=*znak++;             //Pobierz bajt opisu znaku
				}
				if(coldesc & colmask)
				OLED_SetPixel(y + oy, x, tmpFgClr);				//Ró¿ne kombinacje x,y umo¿liwiaj¹ dowolne obroty znaku
					else OLED_SetPixel(y + oy, x, tmpBgClr);	//Ale rysujemy te¿ t³o
				colmask>>=1;
			}
			x++;
			if(x == OLED_WIDTH) return; //Wychodzimy za LCD
		}
	}
}