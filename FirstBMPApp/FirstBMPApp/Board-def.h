/*
 * Board_def.h
 *
 * Created: 2018-09-22 10:11:36
 *  Author: tmf
 */ 


#ifndef BOARD_DEF_H_
#define BOARD_DEF_H_


#include "PinMacros.h"

//Definicje USART
#define USART_BAUD	9600				//Szybkoœæ transmisji danych

//Definicje pinów dla czujnika BMP/E
#define BMP_En		B,4					//Pin wykorzystywany jako Enable dla czujnika BMP/E280
#define BMPP_En		B,5					//Pin wykorzystywany jako Enable dla czujnika BMP/E280


#endif /* BOARD_DEF_H_ */