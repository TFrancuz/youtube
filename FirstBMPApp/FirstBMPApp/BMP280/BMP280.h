/*
 * BMP280.h
 *
 * Created: 2018-09-08 10:11:22
 *  Author: tmf
 */ 


#ifndef BMP280_H_
#define BMP280_H_

#define BMP280_EN	

#include <avr/io.h>

#include "../Board-def.h"

enum
{
	BMP280_REGISTER_DIG_T1              = 0x88,
	BMP280_REGISTER_DIG_T2              = 0x8A,
	BMP280_REGISTER_DIG_T3              = 0x8C,

	BMP280_REGISTER_DIG_P1              = 0x8E,
	BMP280_REGISTER_DIG_P2              = 0x90,
	BMP280_REGISTER_DIG_P3              = 0x92,
	BMP280_REGISTER_DIG_P4              = 0x94,
	BMP280_REGISTER_DIG_P5              = 0x96,
	BMP280_REGISTER_DIG_P6              = 0x98,
	BMP280_REGISTER_DIG_P7              = 0x9A,
	BMP280_REGISTER_DIG_P8              = 0x9C,
	BMP280_REGISTER_DIG_P9              = 0x9E,

	BMP280_REGISTER_CHIPID             = 0xD0,
	BMP280_REGISTER_VERSION            = 0xD1,
	BMP280_REGISTER_SOFTRESET          = 0xE0,

	BMP280_REGISTER_CAL26              = 0xE1,  // R calibration stored in 0xE1-0xF0
	BMP280_STATUS_ADDR					= 0xF3,
	BMP280_REGISTER_CONTROL            = 0xF4,
	BMP280_REGISTER_CONFIG             = 0xF5,
	BMP280_REGISTER_PRESSUREDATA       = 0xF7,
	BMP280_REGISTER_TEMPDATA           = 0xFA
	
#ifdef USE_BME280
	, BME280_REGISTER_HUM_DATA			= 0xFD,		//Rejestr okreœlaj¹cy wilgotnoœæ
	BME280_CTRL_HUM						= 0xF2,		//Rejestr kontrolny przetwornika wilgotnoœci
	BME280_REGISTER_DIG_H1              = 0xA1,
	BME280_REGISTER_DIG_H2              = 0xE1,
	BME280_REGISTER_DIG_H3              = 0xE3,
	BME280_REGISTER_DIG_H4              = 0xE4,
	BME280_REGISTER_DIG_H5              = 0xE5,
	BME280_REGISTER_DIG_H6              = 0xE7,
#endif	
	
} BMP280_Registers;

enum
{
	BMP280_SLEEP_MODE	= 0x00,
	BMP280_FORCED_MODE	= 0x01,
	BMP280_NORMAL_MODE	= 0x03
} BMP280_PowerMode;

enum
{
	BMP280_ODR_0_5_MS	= 0x00,
	BMP280_ODR_62_5_MS	= 0x01,
	BMP280_ODR_125_MS	= 0x02,
	BMP280_ODR_250_MS	= 0x03,
	BMP280_ODR_500_MS	= 0x04,
	BMP280_ODR_1000_MS	= 0x05,
	BMP280_ODR_2000_MS	= 0x06,
	BMP280_ODR_4000_MS	= 0x07
} BMP280_ODR;

enum
{
	BMP280_OS_NONE	= 0x00,
	BMP280_OS_1X	= 0x01,
	BMP280_OS_2X	= 0x02,
	BMP280_OS_4X	= 0x03,
	BMP280_OS_8X	= 0x04,
	BMP280_OS_16X	= 0x05
} BMP280_OverSampling;

enum
{
	BMP280_FILTER_OFF		= 0x00,
	BMP280_FILTER_COEFF_2	= 0x01,
	BMP280_FILTER_COEFF_4	= 0x02,
	BMP280_FILTER_COEFF_8	= 0x03,
	BMP280_FILTER_COEFF_16	= 0x04
} BMP280_Filter;
	
#define BMP280_SOFT_RESET_CMD	0xB6			//Polecenie resetu uk³adu
#define BMP280_CHIPID			0x58			//ID uk³adu BMP280
#define BME280_CHIPID			0x60			//ID uk³adu BME280
#define BMP280_STATUS_measuring 0x08			//1 jeœli uk³ad aktualnie dokonuje pomiaru
#define BMP280_STATUS_im_update	0x01			//1 jeœ³i uk³ad kopiuje dane z NVM

typedef struct
{
	uint16_t dig_T1;
	int16_t  dig_T2;
	int16_t  dig_T3;

	uint16_t dig_P1;
	int16_t  dig_P2;
	int16_t  dig_P3;
	int16_t  dig_P4;
	int16_t  dig_P5;
	int16_t  dig_P6;
	int16_t  dig_P7;
	int16_t  dig_P8;
	int16_t  dig_P9;

#ifdef USE_BME280
	uint8_t  dig_H1;
	int16_t  dig_H2;
	uint8_t  dig_H3;
	int16_t  dig_H4;
	int16_t  dig_H5;
	int8_t   dig_H6;
#endif
	int32_t	 t_fine;
} BMP280_calib_data;

typedef union
{
	struct
	{
		uint8_t spi3w_en	: 1;	//1 -w³¹cza tryb 3-wire SPI
		uint8_t reserved	: 1;
		uint8_t filter		: 3;	//Okreœla sta³¹ czasow¹ filtra IIR
		uint8_t t_sb		: 3;	//Okreœla czas nieaktywnoœci pomiêdzy pomiarami
	};
	uint8_t byte;
} BMP280_config_reg;

typedef union
{
	struct
	{
		uint8_t mode		: 2;	//Tryb pracy/oszczêdzania energii
		uint8_t osrs_p		: 3;	//Okreœla oversampling dla ciœnienia - 0 - wy³¹czony pomiar, 1-5 - oversampling 1-16razy
		uint8_t osrs_t		: 3;	//Okreœla oversampling dla temperatury - 0 - wy³¹czony pomiar, 1-5 - oversampling 1-16razy
	};
	uint8_t byte;
} BMP280_ctrl_meas_reg;

extern BMP280_calib_data *BMP280_calibs;		//Odczytane parametry kalibracyjne uk³adu BMP280

void BMP280_Init();				//Inicjalizacja BMP280 w trybie SPI

static __attribute__((always_inline)) inline void BMP280_CS(_Bool state)		////Aktywuj (false) lub deaktywuj (true) BMP280
{
	if(state) Port(BMP_En).OUTSET = (1 << Pin(BMP_En));
		else Port(BMP_En).OUTCLR = (1 << Pin(BMP_En));
}

void BMP280_readCoefficients();		//Odczytaj kalibracjê uk³adu

void BMP280_ReadRegs(uint8_t reg_addr, void *reg_data, uint8_t len);	//Odczytaj wskazane rejestry (jeœli len>1)
void BMP280_WriteRegs(uint8_t reg_addr, const uint8_t *reg_data, uint8_t len); //Zapisz wskazane rejestry

void BMP280_soft_reset();		//Zresetuj uk³ad
int8_t BMP280_get_status();		//Zwraca stan rejestru stanu uk³adu
uint8_t BMP280_getID();			//Zwróæ ID uk³adu - dla BMP280 0x58

//Poni¿sze funkcje zwracaj¹ ciœnienie lub temperaturê w formacie sta³opozycyjnym, dwie najmniej znacz¹ce cyfry po przecinku (yyy.xx)
int32_t BMP280_comp_temp_32bit();	//Zwróæ odczyt skompensowanej temperatury
uint32_t BMP280_comp_pres_32bit();	//Zwróæ odczyt skompensowanego ciœnienia
uint32_t BMP280_comp_pres_64bit(); //Zwróæ odczyt skompensowanej temperatury

#ifdef USE_BME280
uint32_t BME280_comp_humid_32bit();		//Zwróæ odczyt skompensowanej wilgotnoœci w formacie XX.yyy%
#endif

#endif /* BMP280_H_ */