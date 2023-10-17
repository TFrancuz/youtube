/*
 * OLED.h
 *
 * Created: 2022-04-13 18:53:27
 *  Author: tmf
 */ 


#ifndef OLED_H_
#define OLED_H_

#ifdef OLED_SH1107
#include <OLED\SH10X.h>
#endif

#ifdef OLED_SSD1351
#include <OLED\SSD1351.h>
#endif

#include <stdint.h>
#include <stdbool.h>
#include <SPI.h>

extern uint8_t OLED_X, OLED_Y;				//Ostatnia zapisana pozycja
extern uint16_t OLED_FgColor, OLED_BgColor;	//Kolor t³a i pierwszego planu

typedef union
{
	struct
	{
		uint8_t blue   : 5;
		uint8_t green  : 6;
		uint8_t red    : 5;
	};
	uint16_t word;
} OLED_RGB565;

void OLED_Init();				//Inicjalizacja wyœwietlacza

void OLED_Clear(uint16_t color);		//Wyczyœæ ekran
void OLED_SetPixel(uint8_t x, uint8_t y, uint16_t color);	//Wyœwietl piksel
void OLED_SetPixel18(uint8_t x, uint8_t y, uint32_t color);	//Wyœwietl piksel w g³êbi 6-6-6
void OLED_SetText(uint8_t x, uint8_t y, const char *tekst, const uint8_t __flash * const __flash font[]);		//Wyœwietl tekst
void OLED_SetTextRotate(uint8_t x, uint8_t y, const char *tekst, const uint8_t __flash * const __flash font[], _Bool invert);	//Wyœwietl teks obrócony o 90 stopni
void OLED_SetTextOp(uint8_t x, uint8_t y, const char *tekst, const uint8_t __flash * const __flash font[], _Bool invert);	//Wyœwietla tekst bez przezroczystoœci, invert==true - odwrócone kolory

#endif /* OLED_H_ */