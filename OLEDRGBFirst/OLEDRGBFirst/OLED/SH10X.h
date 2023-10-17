#ifndef SH110X_H_
#define SH110X_H_

//Lista rejestrów kontrolera

#define SH110X_COLUML				0x00
#define SH110X_COLUMH				0x10
#define SH110X_PAGEADDRMODE			0x20
#define SH110X_COLUMNADDRMODE		0x21
#define SH110X_PAGEADDR				0x22
#define SH110X_SETCONTRAST			0x81
#define SH110X_CHARGEPUMP			0x8D
#define SH110X_SEGREMAPOFF			0xA0
#define SH110X_SEGREMAPON			0xA1
#define SH110X_DISPLAYALLOFF		0xA4
#define SH110X_DISPLAYALLON			0xA5
#define SH110X_NORMALDISPLAY		0xA6
#define SH110X_INVERTDISPLAY		0xA7
#define SH110X_SETMULTIPLEX			0xA8
#define SH110X_DCDC					0xAD
#define SH110X_DISPLAYOFF			0xAE
#define SH110X_DISPLAYON			0xAF
#define SH110X_SETPAGEADDR			0xB0
#define SH110X_COMSCANINC			0xC0
#define SH110X_COMSCANDEC			0xC8
#define SH110X_SETDISPLAYOFFSET		0xD3
#define SH110X_SETDISPLAYCLOCKDIV	0xD5
#define SH110X_SETPRECHARGE			0xD9
#define SH110X_SETCOMPINS			0xDA
#define SH110X_SETVCOMDETECT		0xDB
#define SH110X_SETDISPSTARTLINE     0xDC
#define SH110X_RMWSTART			    0xE0
#define SH110X_RMWEND			    0xEE

#define OLED_WIDTH					128		//Szerokoœæ ekranu w pikselach
#define OLED_HEIGHT					128		//Wysokoœæ ekranu w pikselach
#define OLED_VISHEIGHT				64		//Liczba wyœwietlanych pikseli w pionie

#endif /* SH110X_H_ */