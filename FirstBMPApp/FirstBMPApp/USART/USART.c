/*
 * USART.c
 *
 * Created: 2023-04-30 15:32:26
 *  Author: tmf
 */ 

#include <USART\USART.h>
#include <avr/io.h>
#include <Board-def.h>

void UART_Init()
{
	PORTMUX_USARTROUTEA = PORTMUX_USART3_DEFAULT_gc;	//Przerzucamy wyjœcie na piny 0-3
	USART3_BAUD = 64*F_CPU/16/USART_BAUD;
	USART3_CTRLC = USART_CHSIZE_8BIT_gc;	//8 bitów/znak
	PORTB_DIRSET = PIN0_bm;
	USART3_CTRLB = USART_RXEN_bm | USART_TXEN_bm;		//W³¹cz nadajnik i odbiornik UART
}

void SPI_Init()
{
	PORTMUX_USARTROUTEA = PORTMUX_USART1_ALT1_gc;		//Przerzucamy wyjœcie na piny 4-7
	USART1_BAUD = 128;									//Podzia³ FCLK/4 - 5 MHz
	USART1_CTRLC = USART_CMODE_MSPI_gc;					//Master SPI, NSB first
	PORTC_DIRSET = PIN4_bm;
	PORTC_DIRSET = PIN6_bm;
	USART1_CTRLB = USART_RXEN_bm | USART_TXEN_bm;		//W³¹cz nadajnik i odbiornik UART
}

uint8_t SPI_RW_Byte(uint8_t byte)
{
	USART1_STATUS = USART_TXCIF_bm;
	USART1_TXDATAL = byte;						//Wyœlij znak
	while(!(USART1_STATUS & USART_TXCIF_bm));	//Czy znak zosta³ nadany?
	return USART1_RXDATAL;						//Zwróæ odebrany znak
}

uint8_t USART_RecCh()
{
	while(!(USART3_STATUS & USART_RXCIF_bm));	//Zaczekaj a¿ coœ zostanie odebrane
	return USART3.RXDATAL;						//Przy 8-bitowej ramce odczytujemy tlyko RXDATAL
}

void USART_SendCh(uint8_t ch)
{
	while(!(USART3_STATUS & USART_DREIF_bm));	//Czy jest miejsce w buforze?
	USART3_TXDATAL = ch;						//Wyœlij znak
}

_Bool USART_RecChNonBlocking(uint8_t *ch)
{
	if(USART3_STATUS & USART_RXCIF_bm)
	{
		*ch = USART3.RXDATAL;
		return true;
	}
	return false;				//Nie by³o znaku w buforze nadajnika
}

void USART_SendText(char *text)
{
	do{
		if(*text) USART_SendCh(*text);			//Wyœlij znak
	} while(*text++ != 0);			//Pêtla a¿ do napotkania znaku NUL
}
