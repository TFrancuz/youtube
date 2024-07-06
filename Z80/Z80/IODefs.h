/*
 * IODefs.h
 *
 * Created: 12/3/2023 12:19:25 PM
 *  Author: tmf
 */ 


#ifndef IODEFS_H_
#define IODEFS_H_

//PORTB
#define BOARD_LED	PIN3_bm		//LED na p�ytce
#define BOARD_SW	PIN2_bm		//Przycisk na p�ytce

#define DATA_PORT		PORTC		//Pod PC0-PC7 podpi�te s� sygna�y D0-D7
#define ADDR_LO_PORT	PORTD		//Pod PD0-PD7 podpi�te s� linie A0-A7
#define ADDR_HI_PORT	PORTE		//Pod PE0-PE2 podpi�te s� linie A8-A10
//MREQ - PA2, WR - PA3, RD - PA4
//RESET - PA5, BUSRQ - PA6,  BUSACK - PA7
#define CTRL_MREQ	A,2
#define CTRL_WR		A,3
#define CTRL_RD		A,4
#define CTRL_RESET	A,5
#define CTRL_BUSRQ	A,6
#define CTRL_BUSACK	A,7

#endif /* IODEFS_H_ */