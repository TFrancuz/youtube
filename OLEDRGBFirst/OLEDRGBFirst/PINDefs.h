/*
 * PINDefs.h
 *
 * Created: 2022-04-13 19:10:52
 *  Author: tmf
 */ 


#ifndef PINDEFS_H_
#define PINDEFS_H_

#define OLEDPIN_RESET	PIN5_bm		//Pin Reset
#define OLEDPIN_DC		PIN4_bm		//Pin RS
#define OLEDPIN_CS		PIN3_bm		//Chip Select
#define OLEDPIN_SCK		PIN2_bm		//SCK
#define OLEDPIN_MOSI	PIN0_bm		//MOSI

#define OLED_USART USART3	      //Port USART wykorzystywany do komunikacji z LCD
#define OLED_PORT  PORTB	        //Port do ktorego pod³¹czony jest kontroler

#endif /* PINDEFS_H_ */