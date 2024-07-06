/*
 * Z80.c
 *
 * Created: 6/3/2024 8:36:24 PM
 * Author : tmf
 */ 

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/atomic.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <util\delay.h>
#include "PinMacros.h"
#include "IODefs.h"
#include "UART_Parser.h"
#include "RWAccessh.h"
#include "FLASH.h"

//Domyœlne parametry programowanego uk³adu
IC_Type_t IC_Type = IC_SRAM;				//Typ uk³adu, który testujemy
uint8_t AddrNo = 11, DataNo = 8;			//Parametry programowanego uk³adu
uint8_t Z80_BUSRQ	= 1;						//Nieaktywny stan BUSRQ

//WskaŸniki na funkcje dostêu do pamiêci
Write_t MemWriteFunc;			//WskaŸnik na funkcjê zapisu komórki pamiêci (adres, wartoœæ)
Read_t MemReadFunc;				//WskaŸñik na funkcjê odczytu komórki pamiêci (adres)
ReadFast_t MemFastReadFunc;		//WskaŸnik na funkcjê odczytu komórki pamiêci spod ostatniego u¿ytego adresu

//Makro umieszczaj¹ce zadany ³añcuch w przestrzeni adresowej __flash
#define PGM_STR(X) ((const char[]) { X })

const char msg_UnknownCmd[];
const char msg_OK[] = PGM_STR("OK\r\n");
const char msg_UnknownCmd[] = PGM_STR(" - unknown command\r\n");		//Nie rozpoznane polecenie przes³ane z PC
const char msg_Err[] = PGM_STR("Err\r\n");
const char msg_InvalidArgument[] = PGM_STR("Invalid argument\r\n");
const char mgs_OutOfRange[] = PGM_STR("Argument out of range\r\n");

typedef _Bool (*CmdHandler)(char *param, char **last);

typedef struct
{
	const char *cmd;                //WskaŸnik do polecenia w formacie ASCIIZ
	const CmdHandler  Handler;      //Funkcja realizuj¹ca polecenie
} Command;

_Bool Cmd_A(char *param, char **last);			//Ustaw liczbê linii adresowych
_Bool Cmd_SRAM(char *param, char **last);		//Ustaw typ pamiêci SRAM
_Bool Cmd_FLASH(char *param, char **last);		//Ustaw typ pamiêci FLASH
_Bool Cmd_Suspend(char *param, char **last);	//Aktywuj BUSRQ i zawieœ pracê Z80
_Bool Cmd_Reset(char *param, char **last);		//Zresetuj Z80

//Funkcja pobiera jeden argument z ci¹gu i zwraca go jako uint8_t, zwraca true jeœli ok, false, jeœli konwersja siê nie powiod³a
_Bool GetUInt8Argument(char *param, char **last, uint8_t *val)
{
	char* end;
	param=strtok_r(*last, " ,", last);	//Wyszukaj ci¹g rozdzielaj¹cy - pobieramy offset na stronie
	if(param == NULL) return false;     //B³¹d

	*val = strtol(param, &end, 0);		//Pobierz offset na programowanej stronie
	if(*end) return false;	//B³¹d
	return true;
}

_Bool GetUInt16Argument(char *param, char **last, uint16_t *val)
{
	char* end;
	param=strtok_r(*last, " ,", last);	//Wyszukaj ci¹g rozdzielaj¹cy - pobieramy offset na stronie
	if(param == NULL) return false;     //B³¹d

	*val = strtol(param, &end, 0);		//Pobierz offset na programowanej stronie
	if(*end) return false;	//B³¹d
	return true;
}

_Bool GetUInt32Argument(char *param, char **last, uint32_t *val)
{
	char* end;
	param=strtok_r(*last, " ,", last);	//Wyszukaj ci¹g rozdzielaj¹cy - pobieramy offset na stronie
	if(param == NULL) return false;     //B³¹d

	*val = strtol(param, &end, 0);		//Pobierz offset na programowanej stronie
	if(*end) return false;	//B³¹d
	return true;
}

_Bool Cmd_A(char *param, char **last)			//Ustaw liczbê linii adresowych
{
	if(GetUInt8Argument(param, last, &AddrNo) == false)
	{
		USART_SendText(msg_InvalidArgument);
		return false;
	}
	if((AddrNo < 1) || (AddrNo > 11))
	{
		USART_SendText(mgs_OutOfRange);
		return false;
	}
	return true;
}

_Bool Cmd_SRAM(char *param, char **last)		//Ustaw typ pamiêci SRAM
{
	IC_Type = IC_SRAM;
	MemWriteFunc = SRAM_Write;				//Funkcje dostêpu do pamiêci SRAM
	MemReadFunc = SRAM_Read;
	MemFastReadFunc = SRAM_ReadWOAddr;
	return true;
}

_Bool Cmd_FLASH(char *param, char **last)		//Ustaw typ pamiêci FLASH
{
	Cmd_SRAM(param, last);						//FLASH inicjujemy tak jak SRAM
	IC_Type = IC_FLASH;							//Tylko oznaczamy jako FLASH :)
	return true;
}

_Bool Cmd_Suspend(char *param, char **last)
{
	if(GetUInt8Argument(param, last, &Z80_BUSRQ) == false)
	{
		USART_SendText(msg_InvalidArgument);
		return false;
	}	
	return Z80_Suspend(Z80_BUSRQ);		//Wykonaj wskazan¹ operacjê
}

_Bool Cmd_Reset(char *param, char **last)	//Polecenie zresetowania Z80 - na chwilê aktywuje RESET
{
	Z80_RESETSeq();
	return true;			//Bez znaczenia
}

void Z80_CLKInit()
{
	PORTF_DIRSET = PIN4_bm;
	PORTMUX_TCBROUTEA = PORTMUX_TCB0_ALT1_gc;		//WO0 na PF4
	TCB0.CTRLB = TCB_CCMPEN_bm | TCB_CNTMODE_PWM8_gc;
	TCB0.CCMP = 0x0305; //Wype³nienie/Okres
	TCB0.CTRLA = TCB_CLKSEL_DIV1_gc | TCB_DBGRUN_bm | TCB_ENABLE_bm;	
}

void Z80_RESET(_Bool state)
{
	if(state) Port(CTRL_RESET).OUTSET = (1 << Pin(CTRL_RESET));
		else Port(CTRL_RESET).OUTCLR = (1 << Pin(CTRL_RESET));
}

void Z80_BusRQ(_Bool state)
{
	if(state) 
	{
		Port(CTRL_BUSRQ).OUTSET = (1 << Pin(CTRL_BUSRQ));
		Z80_BUSRQ = 1;
	} else 
	{
		Port(CTRL_BUSRQ).OUTCLR = (1 << Pin(CTRL_BUSRQ));
		Z80_BUSRQ = 0;
	}
}

_Bool Z80_WaitForBUSACK()
{
	while(Port(CTRL_BUSACK).IN & (1 << Pin(CTRL_BUSACK)));
	return true;
}

_Bool Z80_Suspend(_Bool state)
{
	if(!state)
	{
		Z80_BusRQ(1);
	} else
	{
		Z80_BusRQ(0);
		return Z80_WaitForBUSACK();	
	}
	return true;
}

void Z80_RESETSeq()	//Zresetuj Z80
{
	Z80_RESET(0);
	_delay_us(100);
	Z80_RESET(1);
}

//Lista rozpoznawanych poleceñ
const Command Polecenia[]={{PGM_STR("-SRAM"), Cmd_SRAM}, {PGM_STR("-FLASH"), Cmd_FLASH},
	{PGM_STR("-A"), Cmd_A}, {PGM_STR("-Suspend"), Cmd_Suspend}, {PGM_STR("-Reset"), Cmd_Reset}, 
	{PGM_STR("-Read"), ReadMemory}, {PGM_STR("-Write"), FLASHWrite}, {PGM_STR("-ID"), FLASHReadSignature}};

void InterpretCommand(char *cmdline)
{
	_Bool retVal = false;
	uint8_t indeks;
	char *last = cmdline;
	uint8_t max_indeks=sizeof(Polecenia)/sizeof(Polecenia[0]);
	char *cmd;
	do{
		cmd = strtok_r(last, " ", &last); //Wydziel polecenie z przekazanej linii
		if(cmd != NULL) 		//Jeœli znaleziono poprawny format polecenia, to spróbujmy je wykonaæ
		{
			for(indeks = 0; indeks < max_indeks; indeks++)
			{
				if(strcmp(cmd, Polecenia[indeks].cmd) == 0) //Przeszukaj listê poleceñ
				{
					retVal = Polecenia[indeks].Handler(last, &last);   //Wywo³aj funkcjê obs³ugi przes³anego polecenia
					break;
				}
			}
			if(indeks == max_indeks)  //Jeœli polecenie nieznane lub b³¹d...
			{
				USART_SendText(cmd);			//Wyœlij polecenie, które spowodowa³o b³¹d
				USART_SendText(msg_UnknownCmd); //B³¹d - nieznane polecenie
				break;
			}
		}
	} while((cmd) && (retVal));
	if((cmd == NULL) && (retVal == true)) USART_SendText(msg_OK);
}

int main(void)
{
	//Domyœlnie MCU startuje z wew. zegarem 4 MHz
	CPU_CCP = CCP_IOREG_gc;							//Odblokuj dostêp do rejestru chronionego
	CLKCTRL.OSCHFCTRLA = CLKCTRL_FRQSEL_24M_gc;		//Zegar 24 MHz
	
	IO_CPUCTRL_Init();		//Inicjalizacja pinów IO po³¹czonych z Z80
	IO_Bus_Off();			//Linie kontrolne pamiêci, szyny danych i adresowe od strony AVR s¹ wy³¹czone
	Z80_CLKInit();			//Inicjalizacja zegara taktuj¹cego Z80
	UART_Init();			//Inicjalizacja USART
	sei();					//Odblokowjemy przerwania

	Cmd_SRAM(NULL, NULL);		//Domyœlnie obs³ugujemy SRAM

    while (1) 
    {
		if(CmdReceived)
		{
			CmdReceived = false;
			InterpretCommand(RxBuffer);			//Zdekoduj przes³ane polecenie
		}
    }
}

