/*
 * UARTInt.h
 *
 * Created: 12/2/2023 3:57:01 PM
 *  Author: tmf
 */ 


#ifndef UARTINT_H_
#define UARTINT_H_
#include <stdint.h>

#define BAUDRATE 9600
#define MAX_RXBUFFER_SIZE 256
#define MAX_TXBUFFER_SIZE 256

extern char RxBuffer[MAX_RXBUFFER_SIZE];
extern uint8_t TxBuffer[MAX_TXBUFFER_SIZE];
extern volatile _Bool CmdReceived;

void UART_Init();						//Inicjalizacja UART
void USART_SendCh(uint8_t ch);			//Wyœlij jeden znak
void USART_SendText(const char *text);	//Wyœlij tekst

#endif /* UARTINT_H_ */