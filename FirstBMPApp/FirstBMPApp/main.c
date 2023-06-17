/*
 * FirstBMPApp.c
 *
 * Created: 2023-04-30 15:26:34
 * Author : tmf
 */ 


#include <USART/USART.h>
#include <BMP280/BMP280.h>
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>


void SendBMPReadings()
{	
	char buf[255];

	int32_t temperature = BMP280_comp_temp_32bit();
	uint32_t press = BMP280_comp_pres_32bit();
#ifdef USE_BME280
	uint32_t humid = BME280_comp_humid_32bit();
	sprintf(buf,"Temperatura: %ld, cisnienie: %lu, wilgotnosc: %lu\r\n", temperature, press, humid);
#else
	sprintf(buf,"Temperatura: %ld, cisnienie: %lu\r\n", temperature, press);
#endif
	USART_SendText(buf);	//Wyœlij dane do PC
	
}

int main(void)
{
	CCP = CCP_IOREG_gc;
	CLKCTRL_MCLKCTRLB = 0;	//20 MHz
	
	UART_Init();		//Inicjalizacja USART
	SPI_Init();			//Inicjalizacja interfejsu dla BMP280
	BMP280_Init();		//Inicjalizacja uk³adu BMP
	
    /* Replace with your application code */
    while (1) 
    {
		SendBMPReadings();
		_delay_ms(1000);
    }
}

