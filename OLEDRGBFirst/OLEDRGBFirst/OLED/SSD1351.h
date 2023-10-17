/*
 * SSD1351.h
 *
 * Created: 2022-04-15 18:23:57
 *  Author: tmf
 */ 


#ifndef SSD1351_H_
#define SSD1351_H_

#define SSD1351_CMD_SETCOLUMN			0x15
#define SSD1351_CMD_SETROW				0x75 
#define SSD1351_CMD_WRITERAM			0x5C
#define SSD1351_CMD_READRAM				0x5D
#define SSD1351_CMD_SETREMAP			0xA0
#define SSD1351_CMD_STARTLINE			0xA1
#define SSD1351_CMD_DISPLAYOFFSET		0xA2
#define SSD1351_CMD_DISPLAYALLOFF		0xA4
#define SSD1351_CMD_DISPLAYALLON		0xA5
#define SSD1351_CMD_NORMALDISPLAY		0xA6
#define SSD1351_CMD_INVERTDISPLAY		0xA7
#define SSD1351_CMD_FUNCTIONSELECT		0xAB
#define SSD1351_CMD_DISPLAYOFF			0xAE
#define SSD1351_CMD_DISPLAYON			0xAF
#define SSD1351_CMD_PRECHARGE			0xB1
#define SSD1351_CMD_DISPLAYENHANCE		0xB2
#define SSD1351_CMD_CLOCKDIV			0xB3
#define SSD1351_CMD_SETVSL				0xB4  
#define SSD1351_CMD_SETGPIO				0xB5
#define SSD1351_CMD_PRECHARGE2			0xB6 
#define SSD1351_CMD_SETGRAY				0xB8
#define SSD1351_CMD_USEDEFAULTLUT		0xB9
#define SSD1351_CMD_PRECHARGELEVEL		0xBB
#define SSD1351_CMD_VCOMH				0xBE
#define SSD1351_CMD_CONTRASTABC			0xC1
#define SSD1351_CMD_CONTRASTMASTER		0xC7
#define SSD1351_CMD_MUXRATIO			0xCA
#define SSD1351_CMD_COMMANDLOCK			0xFD
#define SSD1351_CMD_HORIZSCROLL			0x96
#define SSD1351_CMD_STOPSCROLL			0x9E
#define SSD1351_CMD_STARTSCROLL			0x9F

#define OLED_WIDTH					128		//Szeroko�� ekranu w pikselach
#define OLED_HEIGHT					128		//Wysoko�� ekranu w pikselach
#define OLED_VISHEIGHT				128		//Liczba wy�wietlanych pikseli w pionie

#endif /* SSD1351_H_ */