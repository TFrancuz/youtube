/*
 * SRAMTester.c
 *
 * Created: 10/1/2023 1:23:40 PM
 * Author : tmf
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <util\delay.h>

#define BAUD 9600
#define MAX_RXBUFFER_SIZE 256
#define MAX_TXBUFFER_SIZE 256

char RxBuffer[MAX_RXBUFFER_SIZE];
uint8_t RxBufReadIndex = 0, RxBufferWriteIndex = 0;
volatile uint8_t RxBufferCnt = 0;

uint8_t TxBuffer[MAX_TXBUFFER_SIZE];
uint8_t TxBufferWriteIndex = 0, TxBufferReadIndex = 0;
volatile uint8_t TxBufferCnt = 0;
volatile _Bool TxIntEn;
volatile _Bool CmdReceived = false;

ISR(USART3_RXC_vect)
{
	uint8_t recData = USART3_RXDATAL;
	if((CmdReceived == false) && (RxBufferWriteIndex < sizeof(RxBuffer)))	//Odbieramy znaki tylko jeœli jest pusty bufor poleceñ i jest wolne miejsce
	{
		RxBuffer[RxBufferWriteIndex] = recData;		//Zapisz odczytany znak do bufora
		RxBufferCnt++;
		if(RxBuffer[RxBufferWriteIndex] == '\n')
		{
			if((RxBufferWriteIndex >= 1) && (RxBuffer[RxBufferWriteIndex - 1] == '\r')) 
			{
				--RxBufferWriteIndex;
				--RxBufferCnt;
			}
			RxBuffer[RxBufferWriteIndex] = 0;	//Wpisz koñcowy znak NUL
			CmdReceived = true;
			RxBufferCnt = 0; RxBufferWriteIndex = 0;
		} else RxBufferWriteIndex++;
	}
}

ISR(USART3_DRE_vect)
{
	while((TxBufferCnt) && (USART3_STATUS & USART_DREIF_bm))	//Zapisujemy a¿ skoñczy siê miejsce w buforze
	{															//lub zabraknie danych do nadania
		USART3_TXDATAL = TxBuffer[TxBufferReadIndex++];
		TxBufferReadIndex%=sizeof(TxBuffer);
		TxBufferCnt--;		
	}
	if(TxBufferCnt == 0) USART3_CTRLA &= ~USART_DREIF_bm;		//nie ma nic do nadania, wiêc blokujemy przerwanie
}

void UART_Init()
{
	TxBufferWriteIndex = 0; TxBufferReadIndex = 0; TxBufferCnt = 0; TxIntEn = false;
	RxBufferWriteIndex = 0; RxBufReadIndex = 0; RxBufferCnt = 0;
	USART3_BAUD = 64*F_CPU/16/BAUD;
	USART3_CTRLC = USART_CHSIZE_8BIT_gc;	//8 bitów/znak
	PORTB_DIRSET = PIN0_bm;					//PB0 - TxD, PB1 - RxD
	USART3_CTRLB = USART_RXEN_bm | USART_TXEN_bm;		//W³¹cz nadajnik i odbiornik UART
	USART3_CTRLA = USART_RXCIE_bm;		//Odblokuj przerwania odbiornika, nadajnika DRE odblokujemy jak bêdzie potrzebne
}

void USART_SendCh(uint8_t ch)
{
	TxBuffer[TxBufferWriteIndex++] = ch;
	TxBufferWriteIndex %= sizeof(TxBuffer);
	ATOMIC_BLOCK(ATOMIC_FORCEON) TxBufferCnt++;
	USART3_CTRLA |= USART_DREIE_bm;			//Odblokuj przerwanie DRE
}

void USART_SendText(const char *text)
{
	do{
		if(*text) USART_SendCh(*text);	//Wyœlij znak
	} while(*text++ != 0);				//Pêtla a¿ do napotkania znaku NUL
}

typedef enum {IC_SRAM, IC_DRAM}
	IC_Type_t;

IC_Type_t IC_Type;				//Typ uk³adu, który testujemy
uint8_t AddrNo, RASNo, DataNo;	//Parametry testowanego uk³adu

//Makro umieszczaj¹ce zadany ³añcuch w przestrzeni adresowej __flash
#define PGM_STR(X) ((const char[]) { X })

extern const char msg_UnknownCmd[];
const char msg_OK[] = PGM_STR("OK\r\n");
const char msg_UnknownCmd[] = PGM_STR(" - unknown command\r\n");		//Nie rozpoznane polecenie przes³ane z PC
const char msg_Err[] = PGM_STR("Err\r\n");
const char msg_InvalidArgument[] = PGM_STR("Invalid argument\r\n");
const char mgs_OutOfRange[] = PGM_STR("Argument out of range\r\n");
	
typedef _Bool (*CmdHandler)(char *param, char **last);

typedef struct
{
	const char *cmd;                //WskaŸnik do polecenia w formacie ASCIIZ
	const CmdHandler  Handler;      //Funkcja realizuj¹ca polecenie
} Command;

_Bool Cmd_A(char *param, char **last);			//Ustaw liczbê linii adresowych
_Bool Cmd_D(char *param, char **last);			//Ustaw liczbê linii danych
_Bool Cmd_RAS(char *param, char **last);		//Ustaw liczbê linii adresowych dla sygna³u RAS
_Bool Cmd_SRAM(char *param, char **last);		//Ustaw typ pamiêci SRAM
_Bool Cmd_DRAM(char *param, char **last);		//Ustaw typ pamiêci DRAM

//Funkcja pobiera jeden argument z ci¹gu i zwraca go jako uint8_t, zwraca true jeœli ok, false, jeœli konwersja siê nie powiod³a
_Bool GetUInt8Argument(char *param, char **last, uint8_t *val)
{
	char* end;
	param=strtok_r(*last, " ,", last);	//Wyszukaj ci¹g rozdzielaj¹cy - pobieramy offset na stronie
	if(param == NULL) return false;     //B³¹d

	*val = strtol(param, &end, 0);		//Pobierz offset na programowanej stronie
	if(*end) return false;	//B³¹d
	return true;
}

_Bool Cmd_A(char *param, char **last)			//Ustaw liczbê linii adresowych
{
	if(GetUInt8Argument(param, last, &AddrNo) == false)
	{
		USART_SendText(msg_InvalidArgument);
		return false;
	}
	if((AddrNo < 1) || (AddrNo > 20))
	{
		USART_SendText(mgs_OutOfRange);	
		return false;
	}
	return true;
}

_Bool Cmd_D(char *param, char **last)			//Ustaw liczbê linii danych
{
	if(GetUInt8Argument(param, last, &DataNo) == false) 
	{
		USART_SendText(msg_InvalidArgument);		
		return false;
	}
	if((DataNo < 1) || (DataNo > 8))
	{
		USART_SendText(mgs_OutOfRange);
		return false;
	}
	return true;
}

_Bool Cmd_RAS(char *param, char **last)			//Ustaw liczbê linii adresowych dla sygna³u RAS
{
	if(GetUInt8Argument(param, last, &RASNo) == false)
	{
		USART_SendText(msg_InvalidArgument);
		return false;
	}
	if(RASNo > 8) 
	{
		USART_SendText(mgs_OutOfRange);
		return false;
	}
	return true;
}	

_Bool Cmd_SRAM(char *param, char **last)		//Ustaw typ pamiêci SRAM
{
	IC_Type = IC_SRAM;
	return true;
}

_Bool Cmd_DRAM(char *param, char **last)		//Ustaw typ pamiêci DRAM
{
	IC_Type = IC_DRAM;
	return true;
}

//Lista rozpoznawanych poleceñ
const Command Polecenia[]={{PGM_STR("-SRAM"), Cmd_SRAM}, {PGM_STR("-DRAM"), Cmd_DRAM},
						   {PGM_STR("-A"), Cmd_A}, {PGM_STR("-D"), Cmd_D}, {PGM_STR("-RAS"), Cmd_RAS}};


void InterpretCommand(char *cmdline)
{
	_Bool retVal = false;
	uint8_t indeks;
	char *last = cmdline;
	uint8_t max_indeks=sizeof(Polecenia)/sizeof(Polecenia[0]);
	char *cmd;
	do{	
		cmd = strtok_r(last, " ", &last); //Wydziel polecenie z przekazanej linii
		if(cmd != NULL) 		//Jeœli znaleziono poprawny format polecenia, to spróbujmy je wykonaæ
		{
			for(indeks = 0; indeks < max_indeks; indeks++)
			{
				if(strcmp(cmd, Polecenia[indeks].cmd) == 0) //Przeszukaj listê poleceñ
				{
					retVal = Polecenia[indeks].Handler(last, &last);   //Wywo³aj funkcjê obs³ugi przes³anego polecenia
					break;
				}
			}
		}
		if(indeks == max_indeks)  //Jeœli polecenie nieznane lub b³¹d...
		{
			USART_SendText(cmd);			//Wyœlij polecenie, które spowodowa³o b³¹d
			USART_SendText(msg_UnknownCmd); //B³¹d - nieznane polecenie
			break;
		}
	} while((cmd) && (retVal));
	if((cmd == NULL) && (retVal == true)) USART_SendText(msg_OK);
}

int main(void)
{
	//Domyœlnie MCU startuje z wew. zegarem 4 MHz
	CPU_CCP = CCP_IOREG_gc;							//Odblokuj dostêp do rejestru chronionego
	CLKCTRL.OSCHFCTRLA = CLKCTRL_FRQSEL_24M_gc;		//Zegar 24 MHz
    
	UART_Init();				//Inicjalizacja USART
	sei();						//Odblokowjemy przerwania
	
	PORTB_DIRSET = PIN3_bm;		//Pin steruj¹cy LEDem

    while (1) 
    {
		if(CmdReceived)
		{
			ATOMIC_BLOCK(ATOMIC_FORCEON) CmdReceived = false;
			InterpretCommand(RxBuffer);			//Zdekoduj przes³ane polecenie
		}
    }
}

