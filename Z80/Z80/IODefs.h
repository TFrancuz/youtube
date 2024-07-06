/*
 * IODefs.h
 *
 * Created: 12/3/2023 12:19:25 PM
 *  Author: tmf
 */ 


#ifndef IODEFS_H_
#define IODEFS_H_

//PORTB
#define BOARD_LED	PIN3_bm		//LED na p³ytce
#define BOARD_SW	PIN2_bm		//Przycisk na p³ytce

#define DATA_PORT		PORTC		//Pod PC0-PC7 podpiête s¹ sygna³y D0-D7
#define ADDR_LO_PORT	PORTD		//Pod PD0-PD7 podpiête s¹ linie A0-A7
#define ADDR_HI_PORT	PORTE		//Pod PE0-PE2 podpiête s¹ linie A8-A10
//MREQ - PA2, WR - PA3, RD - PA4
//RESET - PA5, BUSRQ - PA6,  BUSACK - PA7
#define CTRL_MREQ	A,2
#define CTRL_WR		A,3
#define CTRL_RD		A,4
#define CTRL_RESET	A,5
#define CTRL_BUSRQ	A,6
#define CTRL_BUSACK	A,7

#endif /* IODEFS_H_ */