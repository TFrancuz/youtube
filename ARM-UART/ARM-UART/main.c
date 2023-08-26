/*
 * ARM-UART.c
 *
 * Created: 8/14/2023 9:30:49 PM
 * Author : tmf
 */ 

#include "sam.h"
#include <stdbool.h>
#include "Clk/SetClk.h"
#include "Delay/delay.h"

#define SHIFT 32

/*
* \internal Calculate asynchronous baudrate value (UART)
*/
uint16_t calculate_baud_value(const uint32_t baudrate, const uint32_t peripheral_clock, uint8_t sample_num);

uint8_t RxBuffer[256];
uint8_t RxBufReadIndex, RxBufferWriteIndex;
volatile uint8_t RxBufferCnt;

uint8_t TxBuffer[256];
uint8_t TxBufferWriteIndex, TxBufferReadIndex;
volatile uint8_t TxBufferCnt;
volatile _Bool TxIntEn;

void SERCOM3_Handler()
{
	if(SERCOM3->USART.INTFLAG.bit.DRE)		//Wyst¹pi³o przerwniae DRE - mamy miejsce w buforze nadajnika
	{
		while((TxBufferCnt) && (SERCOM3->USART.INTFLAG.bit.DRE))	//Zapisujemy a¿ skoñczy siê miejsce w buforze
		{															//lub zabraknie danych do nadania
			SERCOM3->USART.DATA.reg = TxBuffer[TxBufferReadIndex++];
			TxBufferReadIndex%=sizeof(TxBuffer);
			TxBufferCnt--;
		}
		if(TxBufferCnt == 0) SERCOM3->USART.INTENCLR.reg = SERCOM_USART_INTENSET_DRE;	//nie ma nic do nadania, wiêc blokujemy przerwanie
	}
	
	if(SERCOM3->USART.INTFLAG.bit.RXC)		//Odebraliœmy nowy znak
	{
		RxBuffer[RxBufferWriteIndex] =SERCOM3->USART.DATA.reg;		//Zapisz odczytany znak do bufora
		RxBufferWriteIndex = (RxBufferWriteIndex + 1) % sizeof(RxBuffer);
		RxBufferCnt++;		
	}
}

void UART_init()
{
	//Konfiguracja UART - u¿ywamy SERCOM3 gdy¿ jest wyprowadzony jako VCP
	//Wyjœcia TxD - PA22, RxD - PA23
	REG_PM_APBCMASK |= PM_APBCMASK_SERCOM3;  //W³¹cz zegar dla SERCOM3
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_SERCOM3_CORE_Val) | //Generic Clock 0
	GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_CLKEN;                            // jest Ÿród³em zegara
	
	while(GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY); //Zaczekaj na synchronizacjê
	
	//UART, LSB first, internal clock, Rxt - PAD1, TXD - PAD0
	SERCOM3->USART.CTRLA.reg = SERCOM_USART_CTRLA_DORD | SERCOM_USART_CTRLA_FORM(0) | SERCOM_USART_CTRLA_RXPO(1)
							| SERCOM_USART_CTRLA_TXPO(0) | SERCOM_USART_CTRLA_MODE_USART_INT_CLK;
	SERCOM3->USART.BAUD.reg = calculate_baud_value(9600, 48000000, 16);
	//Odblokuj nadajnik i odbiotrnik, 8 bitowa ramka, bez parzystoœci, 1 bit stopu
	SERCOM3->USART.CTRLB.reg = SERCOM_USART_CTRLB_CHSIZE(0x0) | SERCOM_USART_CTRLB_RXEN | SERCOM_USART_CTRLB_TXEN;
	while(SERCOM3->USART.SYNCBUSY.bit.CTRLB);		//Zaczekaj na synchronizacjê rejestrów

	NVIC_SetPriority(SERCOM3_IRQn, 3);		//Poziom przerwañ
	NVIC_EnableIRQ(SERCOM3_IRQn);			//Odblokuj przerwania
	//Odblokuj przerwanie DRE i RXC USART3
	SERCOM3->USART.INTENSET.reg = SERCOM_USART_INTENSET_RXC;

	SERCOM3->USART.CTRLA.reg |= SERCOM_USART_CTRLA_ENABLE;				//Odblokuj UART
	while(SERCOM3->USART.SYNCBUSY.reg & SERCOM_USART_SYNCBUSY_ENABLE);	//Zaczekaj na koniec operacji
	
	PORT->Group[0].WRCONFIG.reg = PORT_WRCONFIG_HWSEL | PORT_WRCONFIG_WRPINCFG | PORT_WRCONFIG_WRPMUX | PORT_WRCONFIG_PMUX(2) | PORT_WRCONFIG_PMUXEN | 0b11000000; //Wybierz funkcjê SERCOM1 dla PA22-23
}

/*
* internal Calculate 64 bit division, ref can be found in
* http://en.wikipedia.org/wiki/Division_algorithm#Long_division
*/
static uint64_t long_division(uint64_t n, uint64_t d)
{
	int32_t i;
	uint64_t q = 0, r = 0, bit_shift;
	for (i = 63; i >= 0; i--)
	{
		bit_shift = (uint64_t)1 << i;
		r = r << 1;
		if (n & bit_shift) r |= 0x01;
		if (r >= d)
		{
			r = r - d;
			q |= bit_shift;
		}
	}
	return q;
}

/*
* \internal Calculate asynchronous baudrate value (UART)
*/
uint16_t calculate_baud_value(const uint32_t baudrate, const uint32_t peripheral_clock, uint8_t sample_num)
{
	/* Temporary variables */
	uint64_t ratio = 0;
	uint64_t scale = 0;
	uint64_t baud_calculated = 0;
	uint64_t temp1;
	/* Calculate the BAUD value */
	temp1 = ((sample_num * (uint64_t)baudrate) << SHIFT);
	ratio = long_division(temp1, peripheral_clock);
	scale = ((uint64_t)1 << SHIFT) - ratio;
	baud_calculated = (65536 * scale) >> SHIFT;
	return baud_calculated;
}

void USART_SendCh(uint8_t ch)
{
//	while(!(SERCOM3->USART.INTFLAG.reg & SERCOM_USART_INTFLAG_DRE));  //Zaczekaj na wys³anie danych
//	SERCOM3->USART.DATA.reg=ch;
	TxBuffer[TxBufferWriteIndex++] = ch;
	TxBufferWriteIndex%=sizeof(TxBuffer);
	__disable_irq();
	 TxBufferCnt++;		//Sekcja krytyczna
	 __enable_irq();
	SERCOM3->USART.INTENSET.reg = SERCOM_USART_INTENSET_DRE;			//Odblokuj przerwanie DRE
}

void USART_SendText(char *text)
{
	do{
		if(*text) USART_SendCh(*text);			//Wyœlij znak
	} while(*text++ != 0);			//Pêtla a¿ do napotkania znaku NUL
}

int main(void)
{
	Set48MHzClk();
	UART_init();
	__enable_irq();			//Zezwól globalnie na przerwania

	while(1)
	{
		USART_SendText("Hi, this is ARM & interrupts...\r\n");
		_delay_ms(500);
	}	
}

