/*
 * RWAccess.c
 *
 * Created: 12/3/2023 12:15:19 PM
 *  Author: tmf
 */ 

#include "RWAccessh.h"
#include <avr/io.h>
#include "IODefs.h"
#include "PinMacros.h"
#include <util/delay.h>

void IO_CPUCTRL_Init()
{
	Port(CTRL_RESET).DIRSET = (1 << Pin(CTRL_RESET));	//RESET i BUSRQ w stanie nieaktywnym
	Port(CTRL_BUSRQ).DIRSET = (1 << Pin(CTRL_BUSRQ));
	Port(CTRL_BUSACK).DIRCLR = (1 << Pin(CTRL_BUSACK));	//BUSACK jest wejœciem

	Port(CTRL_RESET).OUTSET = (1 << Pin(CTRL_RESET));	//Ale jeœli bêd¹ wyjœciem to domyœlnie w stanie nieaktywnym
	Port(CTRL_BUSRQ).OUTSET = (1 << Pin(CTRL_BUSRQ));	
}

void IO_Parallel_Init()
{
	DATA_PORT.DIR = 0x00;			//D0-D7 s¹ wejœciami
	ADDR_LO_PORT.DIR = 0xFF;		//A0-A7 s¹ wyjœciami
	ADDR_HI_PORT.DIRSET = 0b111;	//A8-A10 s¹ wyjœciami
	Port(CTRL_MREQ).DIRSET = (1 << Pin(CTRL_MREQ));	//MREQ, WR i RD s¹ wyjœciami
	Port(CTRL_RD).DIRSET = (1 << Pin(CTRL_RD));
	Port(CTRL_WR).DIRSET = (1 << Pin(CTRL_WR));
	Port(CTRL_MREQ).OUTSET = (1 << Pin(CTRL_MREQ));	//Ale jeœli bêd¹ wyjœciem to domyœlnie w stanie nieaktywnym
	Port(CTRL_RD).OUTSET = (1 << Pin(CTRL_RD));
	Port(CTRL_WR).OUTSET = (1 << Pin(CTRL_WR));
}

void IO_Bus_Off()
{
	DATA_PORT.DIR = 0x00;			//D0-D7 s¹ wejœciami
	ADDR_LO_PORT.DIR = 0x00;		//A0-A7 s¹ wejœciami
	ADDR_HI_PORT.DIRCLR = 0b111;	//A8-A10 s¹ wejœciami
	Port(CTRL_MREQ).DIRCLR = (1 << Pin(CTRL_MREQ));	//MREQ, WR i RD s¹ wyjœciami
	Port(CTRL_RD).DIRCLR = (1 << Pin(CTRL_RD));
	Port(CTRL_WR).DIRCLR = (1 << Pin(CTRL_WR));
}

void SetAddr(uint32_t addr)
{
	ADDR_LO_PORT.OUT = addr & 0xff;				//Bity 0-7 reprezentuj¹ A0-A7
	ADDR_HI_PORT.OUTCLR = 0b111;				//Tylko bity 0-2 reprezentuj¹ adres A8-A10
	ADDR_HI_PORT.OUTSET = (addr >> 8) & 0b111;
}

//Operacje RW dla pamiêci SRAM
void SRAM_Write(uint32_t addr, uint8_t data)
{
	SetAddr(addr);
	DATA_PORT.DIR = 0xff;
	DATA_PORT.OUT = data;
	Port(CTRL_MREQ).OUTCLR = (1 << Pin(CTRL_MREQ));	//Aktywuj stroby zapisu MREQ i WR
	Port(CTRL_WR).OUTCLR = (1 << Pin(CTRL_WR));
	_delay_us(1);
	Port(CTRL_WR).OUTSET = (1 << Pin(CTRL_WR));
	Port(CTRL_MREQ).OUTSET = (1 << Pin(CTRL_MREQ));	//Deaktywuj stroby zapisu MREQ i WR
}

uint8_t SRAM_Read(uint32_t addr)
{
	SetAddr(addr);
	DATA_PORT.DIR = 0x00;
	Port(CTRL_MREQ).OUTCLR = (1 << Pin(CTRL_MREQ));	//Aktywuj stroby odczytu MREQ i RD
	Port(CTRL_RD).OUTCLR = (1 << Pin(CTRL_RD));
	_delay_us(1);
	uint8_t tmpdata = DATA_PORT.IN;
	Port(CTRL_RD).OUTSET = (1 << Pin(CTRL_RD));
	Port(CTRL_MREQ).OUTSET = (1 << Pin(CTRL_MREQ)); //Deaktywuj stroby odczytu MREQ i RD
	return tmpdata;
}

uint8_t SRAM_ReadWOAddr()
{
	DATA_PORT.DIR = 0x00;
	Port(CTRL_MREQ).OUTCLR = (1 << Pin(CTRL_MREQ));	//Aktywuj stroby odczytu MREQ i RD
	Port(CTRL_RD).OUTCLR = (1 << Pin(CTRL_RD));
	_delay_us(1);
	uint8_t tmpdata = DATA_PORT.IN;
	Port(CTRL_RD).OUTSET = (1 << Pin(CTRL_RD));
	Port(CTRL_MREQ).OUTSET = (1 << Pin(CTRL_MREQ)); //Deaktywuj stroby odczytu MREQ i RD
	return tmpdata;
}

//Operacje na pamiêci FLASH
void FLASH_Write(uint32_t addr, uint8_t data)
{
	SetAddr(addr);
	DATA_PORT.DIR = 0xff;
	DATA_PORT.OUT = data;
	Port(CTRL_MREQ).OUTCLR = (1 << Pin(CTRL_MREQ));	//Aktywuj stroby zapisu MREQ i WR
	Port(CTRL_WR).OUTCLR = (1 << Pin(CTRL_WR));
	_delay_us(1);
	Port(CTRL_WR).OUTSET = (1 << Pin(CTRL_WR));
	Port(CTRL_MREQ).OUTSET = (1 << Pin(CTRL_MREQ));	//Deaktywuj stroby zapisu MREQ i WR
}

uint8_t FLASH_Read(uint32_t addr)
{
	SetAddr(addr);
	DATA_PORT.DIR = 0x00;
	Port(CTRL_MREQ).OUTCLR = (1 << Pin(CTRL_MREQ));	//Aktywuj stroby odczytu MREQ i RD
	Port(CTRL_RD).OUTCLR = (1 << Pin(CTRL_RD));
	_delay_us(1);
	uint8_t tmpdata = DATA_PORT.IN;
	Port(CTRL_RD).OUTSET = (1 << Pin(CTRL_RD));
	Port(CTRL_MREQ).OUTSET = (1 << Pin(CTRL_MREQ)); //Deaktywuj stroby odczytu MREQ i RD
	return tmpdata;
}