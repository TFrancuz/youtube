/*
 * UART.c
 *
 * Created: 2023-04-29 17:47:36
 * Author : tmf
 */ 

#include <avr/io.h>
#include <stdbool.h>

#define F_CPU 20000000UL
#define BAUD 9600

void UART_Init()
{
	USART3_BAUD = 64*F_CPU/16/BAUD;
	USART3_CTRLC = USART_CHSIZE_8BIT_gc;	//8 bitów/znak
	PORTB_DIRSET = PIN0_bm;
	USART3_CTRLB = USART_RXEN_bm | USART_TXEN_bm;		//W³¹cz nadajnik i odbiornik UART
}

uint8_t USART_RecCh()
{
	while(!(USART3_STATUS & USART_RXCIF_bm));	//Zaczekaj a¿ coœ zostanie odebrane
	return USART3.RXDATAL;						//Przy 8-bitowej ramce odczytujemy tlyko RXDATAL
}

void USART_SendCh(uint8_t ch)
{
	while(!(USART3_STATUS & USART_DREIE_bm));	//Czy jest miejsce w buforze?
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
	
    /* Replace with your application code */
    while (1) 
    {
		USART_SendText("Test\r\n");
		switch(USART_RecCh())
		{
			case '1'	:		LED_On();		break;
			case '0'	:		LED_Off();		break;
			case 't'	:		LED_Toggle();	break;
			default		:		USART_SendText("Niezrozumiale polecenie\r\n");	break;
		}
    }
}

