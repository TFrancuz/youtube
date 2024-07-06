/*
 * FLASH.h
 *
 * Created: 12/3/2023 12:20:44 PM
 *  Author: tmf
 */ 


#ifndef FLASH_H_
#define FLASH_H_

#include <stdint.h>
#include <UARTInt.h>
#include <stdbool.h>

_Bool FLASHReadSignature(char *param, char **last);		//Odczytuje sygnaturê pamiêci FLASH
_Bool ReadMemory(char *param, char **last);				//Odczytuje pamiêæ od wskazanego adres, okreœlon¹ liczbê bajtów
														//i przesy³am to przez UART w formacie IntelHEX
_Bool FLASHWrite(char *param, char **last);				//Odczytuje przes³any przez UART plik w formacie IntelHEX
														//i zapisuje jego zawartoœæ do pamiêci FLAH

#endif /* FLASH_H_ */