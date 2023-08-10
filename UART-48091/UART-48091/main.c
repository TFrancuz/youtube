/*
 * UART_48091.c
 *
 * Created: 2023-04-29 17:47:36
 * Author : tmf
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <stdbool.h>
#include <util/delay.h>

#define BAUD 480000

uint8_t RxBuffer[256];
uint8_t RxBufReadIndex, RxBufferWriteIndex;
volatile uint8_t RxBufferCnt;

uint8_t TxBuffer[256];
uint8_t TxBufferWriteIndex, TxBufferReadIndex;
volatile uint8_t TxBufferCnt;
volatile _Bool TxIntEn;

ISR(USART3_RXC_vect)
{
	RxBuffer[RxBufferWriteIndex] = USART3_RXDATAL;		//Zapisz odczytany znak do bufora
	RxBufferWriteIndex = (RxBufferWriteIndex + 1) % sizeof(RxBuffer);
	RxBufferCnt++;
}

ISR(USART3_TXC_vect)
{
	USART3_STATUS |= USART_TXCIF_bm;		//Flaga nie jest automatycznie kasowane po wejœciu w ISR?
	if(TxBufferCnt)
	{
		USART3_TXDATAL = TxBuffer[TxBufferReadIndex++];
		TxBufferReadIndex%=sizeof(TxBuffer);
		TxBufferCnt--;
	} else TxIntEn = false;
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
	USART3_BAUD = 64*F_CPU/16/BAUD;
	USART3_CTRLC = USART_CHSIZE_8BIT_gc;	//8 bitów/znak
	PORTB_DIRSET = PIN0_bm;
	USART3_CTRLB = USART_RXEN_bm | USART_TXEN_bm;		//W³¹cz nadajnik i odbiornik UART_4809
	//USART3_CTRLA = USART_RXCIE_bm | USART_TXCIE_bm;		//Odblokuj przerwania odbiornika i nadajnika UART
	USART3_CTRLA = USART_RXCIE_bm;		//Odblokuj przerwania odbiornika, nadajnika DRE odblokujemy jak bêdzie potrzebne
}

uint8_t USART_RecCh()
{
	while(!(USART3_STATUS & USART_RXCIF_bm));	//Zaczekaj a¿ coœ zostanie odebrane
	return USART3.RXDATAL;						//Przy 8-bitowej ramce odczytujemy tlyko RXDATAL
}

void USART_SendCh(uint8_t ch)
{
//	while(!(USART3_STATUS & USART_DREIE_bm));	//Czy jest miejsce w buforze?
//	USART3_TXDATAL = ch;						//Wyœlij znak
	TxBuffer[TxBufferWriteIndex++] = ch;
	TxBufferWriteIndex%=sizeof(TxBuffer);
	ATOMIC_BLOCK(ATOMIC_FORCEON) TxBufferCnt++;
/*	if(TxIntEn == false)
	{
		TxIntEn = true;
		USART3_TXC_vect();
	}*/
	USART3_CTRLA |= USART_DREIE_bm;			//Odblokuj przerwanie DRE
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

void LED_Init()
{
	PORTF_DIRSET = PIN5_bm;			//Pin IO do którego jest pod³¹czony LED	
}

void LED_On()
{
	PORTF_OUTCLR = PIN5_bm;
}

void LED_Off()
{
	PORTF_OUTSET = PIN5_bm;
}

void LED_Toggle()
{
	PORTF_OUTTGL = PIN5_bm;	
}

int main(void)
{
	CCP = CCP_IOREG_gc;
	CLKCTRL_MCLKCTRLB = 0;		//20 MHz
	
	UART_Init();				//Inicjalizacja USART
	LED_Init();					//LED na module
	sei();						//Odblokowujemy przerwania
	
    /* Replace with your application code */
    while (1) 
    {
		if(TxBufferCnt == 0) USART_SendText("Test0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000\r\n");
		_delay_ms(10);
		uint8_t RecCh;		//Odebrany znak
		if(RxBufferCnt >= 1)		//SprawdŸ czy w buforze znajduje siê jakiœ nieodczytany znak?
		{
			RecCh = RxBuffer[RxBufReadIndex++];				//Odczytaj znak z budora
			RxBufReadIndex%=sizeof(RxBuffer);
			ATOMIC_BLOCK(ATOMIC_FORCEON) RxBufferCnt--;		//To musi byæ zrealizowane atomowo
			switch(RecCh)
			{
				case '1'	:		LED_On();		break;
				case '0'	:		LED_Off();		break;
				case 't'	:		LED_Toggle();	break;
				default		:		USART_SendText("Niezrozumiale polecenie\r\n");	break;
			}
		}
    }
}

