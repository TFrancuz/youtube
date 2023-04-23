/*
 * AY38910-test.c
 *
 * Created: 2022-11-22 22:45:21
 * Author : tmf
 */ 

#include <avr/io.h>
#include <util\delay.h>

#define AY_BDIR			PINC3
#define AY_BC1			PINC1
#define AY_BC2			PINC2
#define AY_RESET		PINC4
#define AY_CLOCK		PINB1

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

void AY_RESETFunc()
{
	PORTC &= ~(1 << AY_RESET);
	_delay_us(1);		//Uk³ad wymaga co najmniej 500 ns
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
	DDRD = 0xff;		//Port danych jako wyjœcie
	PORTD = data;	
}

uint8_t AY_Get8bit()
{
	DDRD = 0x00;		//Port danych jako wejœcie
	return PIND;
}

void AY_Inactive()		//Ustaw piny steruj¹ce tak, aby deaktywowaæ uk³ad AY
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
	AY_Get8bit();			//Tylko, ¿eby ustawiæ port jako wejœcie
	AY_SetBC2(1);
	AY_SetBC1(1);
	_delay_us(1);			//Cykl zapisu trwa max 500 ns
	uint8_t data = AY_Get8bit();
	AY_SetBC1(0);
	AY_SetBC2(0);
	return data;
}

void AY_InitClk()		//Timer 1 generuje sygna³ Clock dla AY na pinie PB1
{
	TCCR1A = 1 << COM1A0;	//Zmieñ na przeciwny OC1A przy ka¿dym overflow
	OCR1A = 4;				//Podziel CPUCLK/8
	TCCR1B = (1 << WGM12) | (1 << CS10);	//Tryb CTC, no-preskaling
	DDRB |= 1 << PINB1;		//Wyjœcie OCA1 timera 
}

void AudioTest()
{
	AY_SelectRegister(REG_FREQ_NOISE);	//Rejestr szumu
	AY_SendDataToRegister(31);
	AY_SelectRegister(REG_IO_MIXER);	//Rejestr kontrolny
	AY_SendDataToRegister(0b11111000);	//W³¹cz kana³y A-C
	AY_SelectRegister(REG_LVL_A);
	AY_SendDataToRegister(31);
	AY_SelectRegister(REG_LVL_B);
	AY_SendDataToRegister(31);
	AY_SelectRegister(REG_LVL_C);
	AY_SendDataToRegister(31);
	AY_SelectRegister(REG_ENV_SHAPE);				//Envelope
	AY_SendDataToRegister(0b1100);//(0b1010);
	AY_SelectRegister(REG_FREQ_ENV_HI);				//Envelope
	AY_SendDataToRegister(0b1100);
	AY_SelectRegister(REG_FREQ_ENV_LO);				//Envelope
	AY_SendDataToRegister(0b1100);
	AY_SelectRegister(REG_FREQ_A_LO);				//Tone A
	AY_SendDataToRegister(0b1100);
	AY_SelectRegister(REG_FREQ_A_HI);				//Tone A
	AY_SendDataToRegister(0b1100);

	AY_SelectRegister(REG_FREQ_B_LO);			//Tone B
	AY_SendDataToRegister(0b1100);	
	AY_SelectRegister(REG_FREQ_B_HI);			//Tone B
	AY_SendDataToRegister(0b1100);	
}

int main(void)
{
	DDRC = 0b00111110;		//Ustaw piny RESET, BDIR, BC1, BC2, CLOCK jako wyjœcia
	AY_Inactive();			//Pocz¹tkowy stan pinów steruj¹cych AY
	AY_InitClk();			//Ustaw zegar taktuj¹cy AY
	AY_RESETFunc();			//Zresetuj AY
	AY_SelectRegister(0x07);
	AY_SendDataToRegister(0b11000000);		//Porty A i B jao wyjœcia

	AudioTest();
	while(1);

    uint8_t cnt = 1;
	while (1) 
    {
		AY_SelectRegister(14);
		AY_SendDataToRegister(cnt);
		AY_SelectRegister(15);
		AY_SendDataToRegister(cnt);
		_delay_ms(1000);
		cnt++;
    }
}

