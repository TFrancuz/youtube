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

_Bool FLASHReadSignature(char *param, char **last);		//Odczytuje sygnatur� pami�ci FLASH
_Bool ReadMemory(char *param, char **last);				//Odczytuje pami�� od wskazanego adres, okre�lon� liczb� bajt�w
														//i przesy�am to przez UART w formacie IntelHEX
_Bool FLASHWrite(char *param, char **last);				//Odczytuje przes�any przez UART plik w formacie IntelHEX
														//i zapisuje jego zawarto�� do pami�ci FLAH

#endif /* FLASH_H_ */