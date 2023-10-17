#ifndef SPI_H_
#define SPI_H_

#include <avr/io.h>
#include <util/delay.h>
#include <PINDefs.h>

__attribute__((always_inline)) inline void OLED_DC(_Bool state)
{
	if(state) OLED_PORT.OUTSET=OLEDPIN_DC;
		else OLED_PORT.OUTCLR=OLEDPIN_DC;
}

__attribute__((always_inline)) inline void OLED_CS(_Bool state)
{
	if(state) OLED_PORT.OUTSET=OLEDPIN_CS;
		else OLED_PORT.OUTCLR=OLEDPIN_CS;
}

static inline void OLED_RESET()
{
	OLED_PORT.OUTSET = OLEDPIN_RESET;
	_delay_ms(1);
	OLED_PORT.OUTCLR = OLEDPIN_RESET;
	_delay_us(2);               //Sygna³ reset musi trwaæ >=2 us
	OLED_PORT.OUTSET = OLEDPIN_RESET;
	_delay_us(2);               //Sygna³ reset musi trwaæ >=2 us
}

__attribute__((always_inline)) inline void SPI_Write_Byte(uint8_t byte)
{
	OLED_USART.TXDATAL=byte;
	while(!(OLED_USART.STATUS & USART_TXCIF_bm));
	OLED_USART.STATUS=USART_TXCIF_bm;
}

__attribute__((always_inline)) inline void OLED_Send_Cmd(uint8_t reg)
{
	OLED_CS(0);
	OLED_DC(0);
	SPI_Write_Byte(reg);
	OLED_DC(1);
	OLED_CS(1);
}

__attribute__((always_inline)) inline void OLED_Send_CmdWithByte(uint8_t reg, uint8_t byte)
{
	OLED_CS(0);
	OLED_DC(0);
	SPI_Write_Byte(reg);
	OLED_DC(1);
	SPI_Write_Byte(byte);
	OLED_CS(1);
}

__attribute__((always_inline)) inline void OLED_Send_CmdWithBytes(uint8_t reg, uint8_t bytes, uint8_t data[])
{
	OLED_CS(0);
	OLED_DC(0);
	SPI_Write_Byte(reg);
	OLED_DC(1);
	for(uint8_t i = 0; i < bytes; i++)
		SPI_Write_Byte(data[i]);
	OLED_CS(1);
}

__attribute__((always_inline))  static inline void OLED_ClearTXCIF()
{
	OLED_USART.STATUS=USART_TXCIF_bm;
}

__attribute__((always_inline))  static inline void OLED_WaitForSent()
{
	while(!(OLED_USART.STATUS & USART_TXCIF_bm));
}

__attribute__((always_inline))  static inline void OLED_SendDataByte(uint8_t data)
{
	while(!(OLED_USART.STATUS & USART_DREIF_bm));
	OLED_USART.TXDATAL=data;
}

void OLED_Interface_Init();		//Inicjalizacja OLED

#endif /* SPI_H_ */