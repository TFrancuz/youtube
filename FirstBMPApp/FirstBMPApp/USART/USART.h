/*
 * USART.h
 *
 * Created: 2023-04-30 15:32:12
 *  Author: tmf
 */ 


#ifndef USART_H_
#define USART_H_

#include <stdint.h>
#include <stdbool.h>

void UART_Init();					//Inicjalizacja USART3, 9600 bps
uint8_t USART_RecCh();				//Zwraca odebrany znak - blokuj�ca
_Bool USART_RecChNonBlocking(uint8_t *ch);	//Zwraca odebrany znak - nieblokuj�ca, false je�li nic nie by�o
void USART_SendCh(uint8_t ch);		//Wysy�a znak
void USART_SendText(char *text);	//Wysy�a tekst w formacie ASCIZ

//Inicjalizacja USART w trybie SPI
void SPI_Init();					//Zainicjuj SPI na USART

uint8_t SPI_RW_Byte(uint8_t byte);


#endif /* USART_H_ */