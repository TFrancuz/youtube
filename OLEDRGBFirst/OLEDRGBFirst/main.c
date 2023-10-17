/*
 * OLEDRGBFirst.c
 *
 * Created: 2022-04-02 22:31:32
 * Author : tmf
 */ 

#include <avr/io.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <Menu\menu.h>

#include "SPI.h"
#include "OLED\OLED.h"

extern const __flash uint8_t* const __flash system16_array[];
extern const __flash uint8_t* const __flash system12_array[];
extern const __flash uint8_t* const __flash system8_array[];

void OLED_Interface_Init()
{
	OLED_PORT.OUT=OLEDPIN_CS | OLEDPIN_DC | OLEDPIN_RESET;  //Deaktywujemy kontroler i inne urz¹dzenia na magistrali SPI
	OLED_PORT.DIR=OLEDPIN_CS | OLEDPIN_DC | OLEDPIN_RESET | OLEDPIN_SCK | OLEDPIN_MOSI; //Ustaw odpowiednie piny jako wyjœcia

	OLED_USART.BAUD=2<<6;	                    //FSCK=FCPU/4 - maksymalne taktowanie SPI 5 MHz
	OLED_USART.CTRLC=USART_CMODE_MSPI_gc;       //Tryb SPI 0
	OLED_USART.CTRLB=USART_TXEN_bm;				//MISO zbêdne bo nie ma na module
}

volatile uint8_t Buttons;         //Stan przycisków - 1 wciœniêty, 0 - zwolniony

ISR(TCA0_OVF_vect)
{
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;		//Flaga nie jest kasowana automatycznie
	static uint8_t counters[3];						//Pomocnicze liczniki dla SW0, SW1 i SW2

	uint8_t tmpbtn=0;
	tmpbtn=((PORTF_IN & PIN4_bm) == 0);
	tmpbtn|=(((PORTF_IN & PIN5_bm) == 0) << 1);
	tmpbtn|=(((PORTF_IN & PIN6_bm) == 0) << 2);

	uint8_t btnmask=1;
	for(uint8_t i = 0; i < sizeof(counters)/sizeof(counters[0]); i++)
	{
		if(counters[i]==0)              //Nie eliminujemy aktualnie drgañ
		{
			if((tmpbtn ^ Buttons) & btnmask)
			{
				Buttons=(Buttons & (~btnmask)) | (tmpbtn & btnmask); //Przepisz stan klawisza
				counters[i]=20;	//Czas przez jaki stan przycisku bêdzie ignorowany
			}
		} else --counters[i];  //Zmniejszaj licznik do 0
		btnmask<<=1;
	}
}

void Kbd_init()
{
	PORTF_DIRCLR = PIN6_bm | PIN5_bm | PIN4_bm;       //W³aœciwie niepotrzebne, gdy¿ domyœlnie te piny s¹ wejœciem, przycisk SW0
	PORTF.PIN4CTRL = PORT_PULLUPEN_bm;		//Pull upy
	PORTF.PIN5CTRL = PORT_PULLUPEN_bm;		//Pull upy
	PORTF.PIN6CTRL = PORT_PULLUPEN_bm;		//Pull upy
	TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc;       //Zwyk³y tryb pracy timera
	TCA0.SINGLE.PER = (F_CPU/1000)-1;              //Sprawdzamy klawisze co oko³o 1 ms
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;		//Przerwanie nadmiaru
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc | TCA_SINGLE_ENABLE_bm;
}

int main(void)
{
	static uint8_t OldButtons;  //Poprzedni stan klawiszy
	CCP=CCP_IOREG_gc;			//Odblokuj dostêp do rejestru
	CLKCTRL.MCLKCTRLB = 0;		//Disable PEN, CLK = 20 MHz
	OLED_Interface_Init();
	OLED_Init();
  
	Kbd_init();              //Obs³uga przycisków

	sei();					 //W³¹czamy przerwania
	 
	 
	OLED_Clear(0x0000);
	OLED_FgColor = 0b1111111111100000;
	OLED_BgColor = 0;//0b0000000000011111;
	
/*while(1)		//On/Off ca³ego wyœwietlacza
{
	OLED_Send_Cmd(SSD1351_CMD_DISPLAYALLON); 
	_delay_ms(1000);
	OLED_Send_Cmd(SSD1351_CMD_DISPLAYALLOFF);
	_delay_ms(1000);
}*/

/*	while(1)		//Prostok¹ty w koloroach R, G i B
	{
		OLED_Clear(0b1111100000000000);
		_delay_ms(1000);
		OLED_Clear(0b0000011111100000);
		_delay_ms(1000);
		OLED_Clear(0b0000000000011111);
		_delay_ms(1000);
	}*/

	for(uint8_t i = 0; i < OLED_WIDTH; i++)
		for(uint8_t y = 0; y < 16; y++)
			{
				OLED_SetPixel(i, y + 0, (OLED_RGB565){.red = (i % 32), .green = 0, .blue = 0}.word);
				OLED_SetPixel(i, y + 20, (OLED_RGB565){.red = 0, .green = (i % 64), .blue = 0}.word);
				OLED_SetPixel(i, y + 40, (OLED_RGB565){.red = 0, .green = 0, .blue = (i % 32)}.word);
			}
	OLED_Send_CmdWithByte(SSD1351_CMD_SETREMAP, 0b10100100);	//Tryb 262k kolorów
	for(uint8_t i = 0; i < OLED_WIDTH; i++)
		for(uint8_t y = 0; y < 16; y++)
			{
				OLED_SetPixel18(i, y + 64, ((uint32_t)i));
				OLED_SetPixel18(i, y + 84, ((uint32_t)i) << 8);
				OLED_SetPixel18(i, y + 104, ((uint32_t)i) << 16);
			}
	
	while(1)		//Korekcja gamma
	{
		_delay_ms(5000);
		OLED_Send_CmdWithBytes(SSD1351_CMD_SETGRAY, 63, (uint8_t[63]){		//Zmieñ korekcjê gamma
			0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
			0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11,
			0x12, 0x13, 0x15, 0x17, 0x19, 0x1B, 0x1D, 0x1F,
			0x21, 0x23, 0x25, 0x27, 0x2A, 0x2D, 0x30, 0x33,
			0x36, 0x39, 0x3C, 0x3F, 0x42, 0x45, 0x48, 0x4C,
			0x50, 0x54, 0x58, 0x5C, 0x60, 0x64, 0x68, 0x6C,
			0x70, 0x74, 0x78, 0x7D, 0x82, 0x87, 0x8C, 0x91,
			0x96, 0x9B, 0xA0, 0xA5, 0xAA, 0xAF, 0xB4});
		_delay_ms(5000);
		OLED_Send_Cmd(SSD1351_CMD_USEDEFAULTLUT);		//Domyœlna liniowa gamma
	}

	while(1);

	while(1)
	{
		OLED_Clear(rand());
	}
	volatile uint8_t Offset = 0;
	OLED_SetTextOp(0, 0, "Hello World!!!", system8_array, false);
	OLED_SetTextOp(0, 20, "Hello World!!!", system12_array, false);
	OLED_SetTextOp(0, 40, "Hello World!!!", system16_array, false);
	while(1)
	{
		uint8_t tmpbtn=Buttons;
		if(((tmpbtn ^ OldButtons) & 4) && (tmpbtn & 4)) 
		{
			Offset++;  //Wciœniêty SW2
			OLED_Send_CmdWithByte(0xA1, Offset);
		}
		OldButtons=tmpbtn;
	};

}

