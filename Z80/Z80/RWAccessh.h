/*
 * RWAccessh.h
 *
 * Created: 12/3/2023 12:15:00 PM
 *  Author: tmf
 */ 


#ifndef RWACCESSH_H_
#define RWACCESSH_H_
#include <stdint.h>

//Typy obs�ugiwanych pami�ci
typedef enum {IC_SRAM, IC_DRAM, IC_FLASH}
IC_Type_t;

extern IC_Type_t IC_Type;					//Typ uk�adu, kt�ry testujemy
extern uint8_t AddrNo, DataNo;				//Parametry testowanego uk�adu
extern uint8_t Z80_BUSRQ;					//Stan sygna�u BUSRQ

//Typy funkcji dost�pu do pami�ci
typedef void (*Write_t)(uint32_t, uint8_t);		//Funkcja zapisu kom�rki pami�ci (adres, warto��)
typedef uint8_t (*Read_t)(uint32_t);			//Funkcja odczytu kom�rki pami�ci (adres)
typedef uint8_t (*ReadFast_t)();				//Funkcja odczytu kom�rki pami�ci spod ostatniego u�ytego adresu

void IO_CPUCTRL_Init();					//Inicjalizacja sygna��w kontrolnych, takich jak RESET, BUSRQ
void IO_Parallel_Init();				//Inicjalizuje piny IO dla konfiguracji pami�ci SRAM i FLASH
void IO_Bus_Off();						//Ustaw wszystkie piny magistrali jako wej�cia
_Bool Z80_Suspend(_Bool state);			//Aktywuje sygna� BUSRQ i oszekuje na potwierdzenie BUSACK - zwraca true je�li wszystko ok
void Z80_RESETSeq();					//Zresetuj CPU

void SetAddr(uint32_t addr);			//Ustawia 20-bitowy adres pami�ci

//Operacje na pami�ci SRAM
void SRAM_Write(uint32_t addr, uint8_t data);	//Zapis wskazanej kom�rki pami�ci
uint8_t SRAM_Read(uint32_t addr);				//Odczyt wskazanej kom�rki pami�ci
uint8_t SRAM_ReadWOAddr();						//Odczyt kom�rki pami�ci na kot�rej ostatnio odbywa�a si� operacja - nie zmienia adresu

//Operacje na pami�ci FLASH
void FLASH_Write(uint32_t addr, uint8_t data);	//Zapis strony pami�ci
uint8_t FLASH_Read(uint32_t addr);				//Odczyt bajtu pami�ci FLASH

#endif /* RWACCESSH_H_ */