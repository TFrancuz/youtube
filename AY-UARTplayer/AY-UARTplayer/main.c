/*
 * AY-UARTplayer.c
 *
 * Created: 2023-05-21 20:59:00
 * Author : tmf
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <util\delay.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define AY_BDIR			PINC3
#define AY_BC1			PINC5
#define AY_BC2			PINC2
#define AY_RESET		PINC4
#define AY_CLOCK		PINB3

#define UART_RTS_PIN	PINB2

uint8_t Buffers[1024];			//Dwa, 512 bajtowe bufory na odbierane dane
uint16_t ActBufferPtr;			//Wska�nik zapisu do aktywnego bufora

volatile uint16_t AYPlayPtr;	//Wska�nik do odtwarzanej pr�bki
volatile int16_t RecDataNo;		//Liczba odebranych bajt�w w buforze
volatile _Bool AYPlay;			//Czy ju� odtwarzamy?

enum
{
	REG_FREQ_A_LO = 0,
	REG_FREQ_A_HI,
	REG_FREQ_B_LO,
	REG_FREQ_B_HI,
	REG_FREQ_C_LO,
	REG_FREQ_C_HI,
	
	REG_FREQ_NOISE,
	REG_IO_MIXER,
	
	REG_LVL_A,
	REG_LVL_B,
	REG_LVL_C,
	
	REG_FREQ_ENV_LO,
	REG_FREQ_ENV_HI,
	REG_ENV_SHAPE,
	
	REG_IOA,
	REG_IOB
};

typedef struct{
uint8_t ID[3];			//Identifier 'PSG'
uint8_t EoT;			//Marker �End of Text� (1Ah)
uint8_t Version;		//Version number
uint8_t Freq;			//Player frequency (for versions 10+)
uint8_t Unknown[10];	//Unknown
} PSF_Header;

#define PSG_IntMarker	0xFF		//Marker bloku op;isu rejestr�w
#define PSG_DelayMarker	0xFE		//Marker ile blok�w nale�y opu�ci�

void AY_RESETFunc()
{
	PORTC &= ~(1 << AY_RESET);
	_delay_us(1);		//Uk�ad wymaga co najmniej 500 ns
	PORTC |= 1 << AY_RESET;
}

void AY_SetBDIR(_Bool state)
{
	if(state) PORTC |= 1<<AY_BDIR;
		else PORTC &= ~(1<<AY_BDIR);
}

void AY_SetBC1(_Bool state)
{
	if(state) PORTC |= 1<<AY_BC1;
		else PORTC &= ~(1<<AY_BC1);
}

void AY_SetBC2(_Bool state)
{
	if(state) PORTC |= 1<<AY_BC2;
		else PORTC &= ~(1<<AY_BC2);
}

void AY_Send8bit(uint8_t data)
{
	DDRD |= 0b11111100;				//Bity 2-7
	uint8_t tmpD = PORTD & 0b00000011;
	PORTD = tmpD | (data & 0b11111100);	
	DDRC |= 0b00000011;				//Bity 0-1
	uint8_t tmpC = PORTC & 0b11111100;
	PORTC = tmpC | (data & 0b00000011);
}

uint8_t AY_Get8bit()
{
	DDRD &= 0b00000011;		//Port danych jako wej�cie - bity 2-7
	DDRC &= 0b11111100;		//Bity 0-1
	return (PIND & 0b11111100) | (PINC & 0b00000011);
}

void AY_Inactive()		//Ustaw piny steruj�ce tak, aby deaktywowa� uk�ad AY
{
	AY_SetBDIR(0);
	AY_SetBC1(0);
	AY_SetBC2(0);
}

void AY_SelectRegister(uint8_t reg)		//Wybierze rejestr AY do zapisu/odczytu
{
	AY_Send8bit(reg);		//Wybrany rejestr
	AY_SetBC1(1);
	_delay_us(2);			//Cykl zapisu trwa min 1800 ns
	AY_SetBC1(0);
}

void AY_SendDataToRegister(uint8_t data)	//Wpisz dane do rejestru AY
{
	AY_Send8bit(data);
	AY_SetBC2(1);
	AY_SetBDIR(1);
	_delay_us(2);			//Cykl zapisu trwa min. 1800 ns
	AY_SetBDIR(0);
	AY_SetBC2(0);
}

uint8_t AY_GetDataFromRegister()	//Odczytaj dane z rejestru AY
{
	AY_Get8bit();			//Tylko, �eby ustawi� port jako wej�cie
	AY_SetBC2(1);
	AY_SetBC1(1);
	_delay_us(1);			//Cykl zapisu trwa max 500 ns
	uint8_t data = AY_Get8bit();
	AY_SetBC1(0);
	AY_SetBC2(0);
	return data;
}

void AY_InitClk()		//Timer 1 generuje sygna� Clock dla AY na pinie PB3
{
	TCCR2A = 1 << COM2A0 | (1 << WGM21);	//Zmie� na przeciwny OC2A przy ka�dym overflow
	OCR2A = 5;				//Podziel CPUCLK/8
	TCCR2B = (1 << CS20);	//Tryb CTC, no-preskaling
	DDRB |= 1 << AY_CLOCK;	//Wyj�cie OC2A timera
}

#define UART_BAUD 57600
#define MYUBRR (F_CPU/16/UART_BAUD-1)
#define BUFFER_TH		900					//Troch� mniej ni� ma bufor, bo RTS nie od razu mo�e zatrzyma� nadajnik

void UART_RTS(_Bool state);		//true - stan aktywny sygna�u RTS
void USART_SendCh(uint8_t ch);
void USART_SendText(char *text);
unsigned char USART_Receive( void );

void USART_Init()
{
	/*Set baud rate */
	UBRR0H = (unsigned char)(MYUBRR>>8);
	UBRR0L = (unsigned char)MYUBRR;
	//Enable receiver and transmitter
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	/* Set frame format: 8data, 1stop bit */
	UCSR0C = (3<<UCSZ00);
	
	//Configure RTS pin
	DDRB |= (1 << UART_RTS_PIN);	//Pin RTS jest wyj�ciem
	UART_RTS(false);				//Dodatkowo RTS jest wyj�ciem w stanie 0
	DDRD |= (1 << PIND1);			//Pin TxD jest wyj�ciem
}

void USART_SendCh(uint8_t ch)
{
	while(!(UCSR0A & (1 << UDRE0)));		//Czy jest miejsce w buforze?
	UDR0 = ch;				//Wy�lij znak
}

void USART_SendText(char *text)
{
	do{
		if(*text) USART_SendCh(*text);			//Wy�lij znak
	} while(*text++ != 0);			//P�tla a? do napotkania znaku NUL
}

unsigned char USART_Receive( void )
{
	/* Wait for data to be received */
	while ( !(UCSR0A & (1<<RXC0)));
	/* Get and return received data from buffer */
	return UDR0;
}

_Bool USART_RecChNonBlocking(uint8_t *ch)
{
	if(UCSR0A & (1<<RXC0))
	{
		*ch = UDR0;
		return true;
	}
	return false;				//Nie by�o znaku w buforze nadajnika
}

void UART_RTS(_Bool state)		//true - stan aktywny sygna�u RTS
{
	if(!state) PORTB &= ~(1 << UART_RTS_PIN);
		else PORTB |= (1 << UART_RTS_PIN);
}

ISR(TIMER1_COMPA_vect)			//Tu odtwarzamy przes�ane z PC info o rejestrach
{
	enum PSG_States {PSG_Nothing, PSG_Data, PSG_DELAY};
	static enum PSG_States State;		//Stan interpretera pliku PSG
	static uint16_t INTDelay;			//Przez ile przerwa� nic nie wysy�amy do AY-3

	if((AYPlay) && (RecDataNo > 0))
	{
		switch(State)
		{
			case PSG_Nothing	:	
									{
										PSF_Header *Header = (PSF_Header *)&Buffers[AYPlayPtr];
										if((memcmp(Header->ID, "PSG", 3) == 0) && (Header->EoT == 0x1a)) 
										{
											State = PSG_Data;
											AYPlayPtr += sizeof(PSF_Header);	//Przeskakujemy nag��wek, wszystko jest ok
											RecDataNo -= sizeof(PSF_Header);
										} else 
										{
											AYPlayPtr++;		//Zobaczmy czy nag��wnek nie znajduje si� od kolejnego bajtu
											RecDataNo--;
										}
									}
									break;
									
			case PSG_Data		:	if(Buffers[AYPlayPtr] == PSG_IntMarker)
									{
										AYPlayPtr++; AYPlayPtr %= sizeof(Buffers); RecDataNo--;
										while((Buffers[AYPlayPtr] != PSG_IntMarker) && (Buffers[AYPlayPtr] != PSG_DelayMarker))
										{
											uint8_t AYReg = Buffers[AYPlayPtr++];	//Pobierz nr rejestru
											AYPlayPtr %= sizeof(Buffers);
											RecDataNo--;
											uint8_t AYData = Buffers[AYPlayPtr++];	//Pobierz dan� do rejestru
											AYPlayPtr %= sizeof(Buffers);
											RecDataNo--;
											if(AYReg < 16)	//Sprawd� czy mamy prawid�owy rejestr, w przeciwnym przypadku zignoruj
											{
												AY_SelectRegister(AYReg);
												AY_SendDataToRegister(AYData);
											}
										}
									} else if(Buffers[AYPlayPtr] == PSG_DelayMarker)
											{
												AYPlayPtr++; AYPlayPtr %= sizeof(Buffers);
												RecDataNo--;
												INTDelay = Buffers[AYPlayPtr];		//Liczba przerwa�, kt�re omijamy bez wpis�w do AY-3 - nale�y pomno�y� razy 4
												INTDelay <<= 2;
												AYPlayPtr++; AYPlayPtr %= sizeof(Buffers);
												RecDataNo--;
												State = PSG_DELAY;
											}
									break;
									
			case PSG_DELAY		:	if(INTDelay) INTDelay--; else State = PSG_Data;	//Nic nie robimy, tylko czekamy a� INTDelay == 0
									break;				
		}

	} else 
	{
		State = PSG_Nothing;
		if(RecDataNo <= 0) 
		{
			AYPlay = false;
			RecDataNo = 0;						//Nie ma �adnych nowych danych do interpretacji
			AYPlayPtr = ActBufferPtr;			//Miejsce, w kt�rym rozpoczynaj� si� nowe dane
			AY_SelectRegister(REG_IO_MIXER);
			AY_SendDataToRegister(0xFF);		//Wycisz d�wi�k z AY-3 - wi�kszo�� plik�w PSG nie wy��cza uk�adu
		}
	}
}

void Timer_Init()			//Inicjalizacja timera odpowiedzialnego za generowanie przerwa� dla AY
{
	TCCR1A = 0;		//CTC Mode
	TCCR1B = (1 << WGM12) | (1 << CS12);	//CTC mode, prescaler /256
	OCR1A = F_CPU/256/50;					//Szybko�� odtwarzania d�wi�ku - normalnie 1 pr�bka co 20ms, 50 pr�bek/s
    TIMSK1 = (1 << OCIE1A);					//Odblokuj przerwania CMP
}

int main(void)
{
	USART_Init();			//Inicjalizacja UART1
	DDRC = (1 << AY_BDIR) | (1 << AY_BC1) | (1 << AY_BC2) | (1 <<AY_RESET);	//Ustaw piny RESET, BDIR, BC1, BC2 jako wyj�cia
	AY_Inactive();			//Pocz�tkowy stan pin�w steruj�cych AY
	AY_RESETFunc();			//Zresetuj AY
	AY_InitClk();			//Timer generuj�cy sygna� CLK dla uk�adu AY - ok. 2 MHz
	Timer_Init();			//Inicjalizacja timera generuj�cego przerwania 50Hz

	sei();					//Odblokuj przerwania

	AYPlayPtr = 0;			//Wska�nik na pocz�tek bufora
	AYPlay = false;			//Na razie nic nie ma, wi�c nic nie odtwarzamy
	ActBufferPtr = 0;		//Wska�nik do bufora
	RecDataNo = 0;			//Pocz�tkowa liczba odebranych bajt�w w buforze

	while (1)
	{
		if(USART_RecChNonBlocking(&Buffers[ActBufferPtr])) 
		{
			ActBufferPtr++;
			ActBufferPtr %= sizeof(Buffers);
			ATOMIC_BLOCK(ATOMIC_FORCEON) RecDataNo++;
		}

		uint16_t tmpChNo;
		ATOMIC_BLOCK(ATOMIC_FORCEON) tmpChNo = RecDataNo;
		if(tmpChNo > BUFFER_TH)	//czy zbli�amy si� do ko�ca bufora?
		{
			UART_RTS(true);			//Zatrzymaj przyjmowanie znak�w
			AYPlay = true;			//Mamy dane, mo�emy je odtwarza�
		} else UART_RTS(false);		//Spoko, odczytujemy dane dalej
	}
}
 