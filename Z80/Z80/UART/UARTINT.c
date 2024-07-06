/*
 * UARTINT.c
 *
 * Created: 12/2/2023 3:56:49 PM
 *  Author: tmf
 */ 

#include <UARTInt.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>
#include <stdbool.h>

char RxBuffer[MAX_RXBUFFER_SIZE];
uint8_t RxBufReadIndex = 0, RxBufferWriteIndex = 0;
volatile uint16_t RxBufferCnt = 0;

uint8_t TxBuffer[MAX_TXBUFFER_SIZE];
uint8_t TxBufferWriteIndex = 0, TxBufferReadIndex = 0;
volatile uint16_t TxBufferCnt = 0;
volatile _Bool TxIntEn;
volatile _Bool CmdReceived = false;

ISR(USART3_RXC_vect)
{
	uint8_t recData = USART3_RXDATAL;
	if((CmdReceived == false) && (RxBufferWriteIndex < sizeof(RxBuffer)))	//Odbieramy znaki tylko jeœli jest pusty bufor poleceñ i jest wolne miejsce
	{
		RxBuffer[RxBufferWriteIndex] = recData;		//Zapisz odczytany znak do bufora
		RxBufferCnt++;
		if(RxBuffer[RxBufferWriteIndex] == '\n')
		{
			if((RxBufferWriteIndex >= 1) && (RxBuffer[RxBufferWriteIndex - 1] == '\r'))
			{
				--RxBufferWriteIndex;
				--RxBufferCnt;
			}
			RxBuffer[RxBufferWriteIndex] = 0;	//Wpisz koñcowy znak NUL
			CmdReceived = true;
			RxBufferCnt = 0; RxBufferWriteIndex = 0;
		} else RxBufferWriteIndex++;
	}
}

ISR(USART3_DRE_vect)
{
	while((TxBufferCnt) && (USART3_STATUS & USART_DREIF_bm))	//Zapisujemy a¿ skoñczy siê miejsce w buforze
	{															//lub zabraknie danych do nadania
		USART3_TXDATAL = TxBuffer[TxBufferReadIndex++];
		TxBufferReadIndex%=sizeof(TxBuffer);
		TxBufferCnt--;
	}
	if(TxBufferCnt == 0) USART3_CTRLA &= ~USART_DREIF_bm;		//nie ma nic do nadania, wiêc blokujemy przerwanie
}

void UART_Init()
{
	TxBufferWriteIndex = 0; TxBufferReadIndex = 0; TxBufferCnt = 0; TxIntEn = false;
	RxBufferWriteIndex = 0; RxBufReadIndex = 0; RxBufferCnt = 0;
	USART3_BAUD = 64*F_CPU/16/BAUDRATE;
	USART3_CTRLC = USART_CHSIZE_8BIT_gc;	//8 bitów/znak
	PORTB_DIRSET = PIN0_bm;					//PB0 - TxD, PB1 - RxD
	USART3_CTRLB = USART_RXEN_bm | USART_TXEN_bm;		//W³¹cz nadajnik i odbiornik UART
	USART3_CTRLA = USART_RXCIE_bm;		//Odblokuj przerwania odbiornika, nadajnika DRE odblokujemy jak bêdzie potrzebne
}

void USART_SendCh(uint8_t ch)
{
	uint16_t tmpcnt;
	do{
		ATOMIC_BLOCK(ATOMIC_FORCEON) tmpcnt = TxBufferCnt;
	} while(tmpcnt == sizeof(TxBuffer));		//Zaczekaj a¿ bêdzie miejsce w buforze
	TxBuffer[TxBufferWriteIndex++] = ch;
	TxBufferWriteIndex %= sizeof(TxBuffer);
	ATOMIC_BLOCK(ATOMIC_FORCEON) TxBufferCnt++;
	USART3_CTRLA |= USART_DREIE_bm;			//Odblokuj przerwanie DRE
}

void USART_SendText(const char *text)
{
	do{
		if(*text) USART_SendCh(*text);	//Wyœlij znak
	} while(*text++ != 0);				//Pêtla a¿ do napotkania znaku NUL
}