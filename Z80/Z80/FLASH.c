/*
 * FLASH.c
 *
 * Created: 12/3/2023 12:21:08 PM
 *  Author: tmf
 */ 

#include "FLASH.h"
#include <string.h>
#include <stdio.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdbool.h>
#include <UARTInt.h>
#include "RWAccessh.h"
#include "UART_Parser.h"

#define FLASH_TOGGLE_BIT		0b01000000			//Bit stanu zapisu FLASH - toggle
#define PAGESIZE	128								//Wielkoœæ strony FLASH
typedef enum {IHEX_Data, IHEX_EOF, IHEX_EXSEGADDR, IHEX_STARTADDR, IHEX_EXLINEARADDR, IHEX_EXLINEARSTART}
				IntelHEX_Cmd_t;

_Bool FLASHReadSignature(char *param, char **last)
{
	char tbl[255];
	IO_Parallel_Init();				//Zainoicjuj piny IO dla bie¿¹cej konfiguracji SRAM
	
	FLASH_Write(0x5555, 0xaa);		//WejdŸ w programowy tryb odczytu sygnatury
	FLASH_Write(0x2aaa, 0x55);
	FLASH_Write(0x5555, 0x90);
	_delay_ms(20);
	uint8_t b1 = FLASH_Read(0x0000);
	uint8_t b2 = FLASH_Read(0x0001);
	sprintf(tbl, "Memory manufacturer 0x%02X, ID: 0x%02X.\r\n", b1, b2);
	USART_SendText(tbl);
	uint8_t l1 = FLASH_Read(0x0002);
	uint8_t l2 = FLASH_Read(0xFFFF2);
	char s1[] = "unlocked"; char s2[] = "unlocked";
	if(l1 == 0xff) strcpy(s1, "locked");
	if(l2 == 0xff) strcpy(s2, "locked");
	sprintf(tbl, "Low bootblock lock: 0x%02X (%s), high bootblock lock: 0x%02X (%s).\r\n", l1, s1, l2, s2);
	USART_SendText(tbl);
	FLASH_Write(0x5555, 0xaa);	//WyjdŸ z trybu odczytu sygnatury - normalna praca FLASH
	FLASH_Write(0x2aaa, 0x55);
	FLASH_Write(0x5555, 0xf0);
	return true;
}

_Bool ReadMemory(char *param, char **last)
{
	uint16_t SegAddr;			//Adres segmentu danych
	uint32_t Addr;				//Adres w ramach segmentu
	char tbl[255], tmpbuf[3];	//Bufor na dane tekstowe
	uint32_t datalen;			//Ile bajtów nale¿y odczytaæ
	uint8_t suma;				//suma kontrolna bloku
	uint8_t ile;

	if(GetUInt32Argument(param, last, &Addr) == false)		//Pobierz adres pocz¹tku czytanego bloku
	{
		USART_SendText(msg_InvalidArgument);
		return false;
	}
	
	if(GetUInt32Argument(param, last, &datalen) == false)	//Pobierz d³ugoœæ czytanego bloku
	{
		USART_SendText(msg_InvalidArgument);
		return false;
	}
	
	Z80_Suspend(true);			//Aktywuje sygna³ BUSRQ i oszekuje na potwierdzenie BUSACK - zwraca true jeœli wszystko ok
	IO_Parallel_Init();			//Zainoicjuj piny IO dla bie¿¹cej konfiguracji SRAM
	
	SegAddr = Addr >> 4;
	
	while(datalen)
	{
		if((Addr & 0xffff) == 0)	//Czy przekraczamy granicê segmentu 64 kB?
		{
			SegAddr = Addr >> 4;	//Uaktualnij adres segmentu
			suma = 0x04 + 0x02 + (SegAddr & 0xff) + ((SegAddr >> 8) & 0xFF);
			uint8_t sumau2 = 0xff - suma + 1;
			sprintf(tbl, ":02000002%02X%02X%02X\r\n", SegAddr >> 8, SegAddr & 0xff, sumau2);	//Wyœlij adres segmentu MSB
			USART_SendText(tbl);
		}
		
		if(datalen > 16) ile = 16; else ile = datalen;			//Ile mamy bajtów do odczytu
		suma = ile + (Addr & 0xff) + ((Addr >> 8) & 0xff);		//Pocz¹tek sumy kontrolnej rekordu
		sprintf(tbl, ":%02X%04X00", ile, (uint16_t)Addr);
		for(uint8_t i = 0; i < ile; i++)
		{
			uint8_t data = FLASH_Read(Addr++);
			suma += data; datalen--;
			sprintf(tmpbuf, "%02X", data);
			strcat(tbl, tmpbuf);
		}
		uint8_t sumau2 = 0xff - suma + 1;
		sprintf(tmpbuf, "%02X", sumau2);
		strcat(tbl, tmpbuf);
		strcat(tbl, "\r\n");
		USART_SendText(tbl);
	}
	USART_SendText(":00000001FF\r\n");	//Rekord end of file

	IO_Bus_Off();				//Prze³¹cz wszystke magistrale AVR w HiZ
	Z80_Suspend(false);			//Odblokuj CPU, nie potrzebujemy RESET bo tylko odczytaliœmy pamiêæ
	return true;
}

_Bool FLASHWrite(char *param, char **last)
{
	_Bool GetByte(char **param, uint8_t *retbyte, _Bool *ok)
	{
		char tmpbuf[3];
		char* end;
		memset(tmpbuf, 0x00, sizeof(tmpbuf));	//Pobierz polecenie
		tmpbuf[0] = **param; (*param)++;
		tmpbuf[1] = **param; (*param)++;
		*retbyte = strtol(tmpbuf, &end, 16);
		if(*end) *ok = false;	//B³¹d
		return *ok;
	}

	_Bool WaitForWriteOk()	//Funkcja sprawdza bit 6 - toogle bit - jeœli kolejne odczyty s¹ ró¿ne to znaczy, ¿e trwa zapis
	{
		uint8_t data1, data2;
		uint8_t delaycnt = 0;
		do{
			data1 = FLASH_Read(0) & FLASH_TOGGLE_BIT;		//Odczytujemy toggle bit
			data2 = FLASH_Read(0) & FLASH_TOGGLE_BIT;
			if(data1 == data2) return true;				//Nie ma zapisu, koñczymy pêtlê
			_delay_us(100);
			delaycnt++;
		} while(delaycnt < 200);					//Warunek przerwania pêtli - time out
		return false;								//Wyst¹pi³ time out - zwracamy b³¹d
	}

	void WritePage(uint32_t Addr, uint8_t *PageData)	//Zapisujemy stronê danych
	{
		WaitForWriteOk();					//Czekamy a¿ zapis bêdzie mo¿liwy
		FLASH_Write(0x5555, 0xaa);			//WejdŸ w tryb programowania strony
		FLASH_Write(0x2aaa, 0x55);
		FLASH_Write(0x5555, 0xA0);
		do{
			FLASH_Write(Addr, *PageData);
			PageData++; Addr++;
		} while(Addr % PAGESIZE);			//Je¿eli == 0 to znaczy, ¿e doszliœmy do koñca strony FLASH - koniec zapisu
		_delay_us(150);		//Brak zmiany WR przez >150 us wymusza zapis strony pamiêci
	}
	
	uint16_t SegAddr = 0;		//Adres segmentu danych
	uint32_t Addr = 0, tmpAddr;	//Adres w ramach segmentu
	uint8_t PageData[PAGESIZE];	//Dane do zaprogramowania strony
	char HEXLine[MAX_TXBUFFER_SIZE];	//Miejsce na odebran¹ liniê poleceñ
	char *HEXLinePtr;			//WskaŸnik na bufor linii HEX
	uint8_t Cmd;
	_Bool ok = true;
	char tbl[255];				//Bufor na dane tekstowe
	uint8_t datalen = 0;		//Ile bajtów odczytano od ostatniej zmiany segmewntu
	uint8_t recdata;			//Liczba bajtów danych w bie¿¹cym rekordzie
	uint32_t totalbytes = 0;	//Ca³kowita liczba odczytanych bajtów danych

	Z80_Suspend(true);			//Aktywuje sygna³ BUSRQ i oszekuje na potwierdzenie BUSACK - zwraca true jeœli wszystko ok
	IO_Parallel_Init();			//Zainoicjuj piny IO dla bie¿¹cej konfiguracji SRAM

	memset(PageData, 0xff, PAGESIZE);	//Zbêdne, ale domyœlnie wartoœæ strony to 0xff
	USART_SendText("Waiting for HEX file...\r\n");
	do{
		while(CmdReceived == false);	//Zaczekaj na odebranie polecenia
		strncpy(HEXLine, RxBuffer, MAX_TXBUFFER_SIZE);	//Skopiuj odebran¹ liniê polecenia
		HEXLinePtr = HEXLine;
		CmdReceived = false;
		if(*HEXLinePtr == ':')		//Znak pocz¹tku linii
		{
			USART_SendText(".");	//Wyœlij znak, ¿e odebraliœmy liniê HEX
			HEXLinePtr++;
			if(GetByte(&HEXLinePtr, &recdata, &ok) == false) break;	//Ile mamy bajtów danych?
			uint8_t tmpval;
			if(GetByte(&HEXLinePtr, &tmpval, &ok) == false) break;  //Adres rekordu - bardziej znacz¹cy bajt
			tmpAddr = tmpval << 8;
			if(GetByte(&HEXLinePtr, &tmpval, &ok) == false) break;	//Adres rekordu - mniej znacz¹cy bajt
			tmpAddr += tmpval;
			tmpAddr = (SegAddr << 4) + tmpAddr;		//Bezwzglêdny adres którego dotyczy operacja
			
			if(GetByte(&HEXLinePtr, &Cmd, &ok) == false) break;	//Pobierz polecenie
			if(((tmpAddr / PAGESIZE) != (Addr / PAGESIZE)) && (datalen > 0))	//Przekraczamy stronê, wiêc star¹ musimy zapisaæ
			{
				WritePage((Addr/PAGESIZE) * PAGESIZE, PageData);
				memset(PageData, 0xff, PAGESIZE);		//Zbêdne, ale domyœlnie wartoœæ strony to 0xff
				datalen = 0; Addr = tmpAddr;
			}
			switch(Cmd) {
				case IHEX_Data:				//Rekord danych
				for(uint8_t i = 0; i < recdata; i++)
				{
					if(GetByte(&HEXLinePtr, &PageData[Addr % PAGESIZE], &ok) == false) break;	//Pobierz dane
					if(((Addr % PAGESIZE) == (PAGESIZE - 1)) && (datalen > 0))
					{
						WritePage((Addr/PAGESIZE) * PAGESIZE, PageData);	//Zapis zaczynamy od pocz¹tku strony
						memset(PageData, 0xff, PAGESIZE);		//Zbêdne, ale domyœlnie wartoœæ strony to 0xff
						datalen = 0;
					}
					Addr++; datalen++; totalbytes++;
				}
				break;
				case IHEX_EOF:				//Koniec pliku
								if(datalen > 0) WritePage((Addr/PAGESIZE) * PAGESIZE, PageData);		//Zapisujemy bie¿¹c¹ stronê pamiêci
								break;
				case IHEX_EXSEGADDR:		//Rozszerzony segment
								if(GetByte(&HEXLinePtr, &tmpval, &ok) == false) break; //Odczytaj bardziej znacz¹cy bajt segmentu
								SegAddr = tmpval << 8;
								if(GetByte(&HEXLinePtr, &tmpval, &ok) == false) break;	//Odczytaj mniej znacz¹cy bajt segmentu
								SegAddr += tmpval;
								break;
				default:			break;
			}
		} else ok = false;		//B³edny znacznik, koñczymy interpretacjê HEXa
	} while((Cmd != IHEX_EOF) && (ok == true));
	if(ok)	sprintf(tbl, "\r\n0x%06lX bytes written.\r\n", totalbytes);
				else sprintf(tbl, "\r\nHEX format error.\r\n");
	USART_SendText(tbl);
	
	IO_Bus_Off();				//Prze³¹cz wszystke magistrale AVR w HiZ
	Z80_Suspend(false);			//Odblokuj CPU
	Z80_RESETSeq();				//Zresetuj Z80 - aby poprawnie wystartowaæ nowy wsad
	USART_SendText("Z80 reset\r\n");	//Info, ¿e zresetowaliœmy Z80
	return ok;
}
