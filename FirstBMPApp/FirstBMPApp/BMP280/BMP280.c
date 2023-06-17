/*
 * BMP280.c
 *
 * Created: 2018-09-08 10:11:59
 *  Author: tmf
 */ 

#include "BMP280.h"
#include "../PinMacros.h"
#include <USART/USART.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdlib.h>

#define BMP280_READ		0x80			//Odczyt rejestru

BMP280_calib_data *BMP280_calibs;		//Odczytane parametry kalibracyjne uk³adu BMP280

void BMP280_Init()
{
	Port(BMP_En).DIRSET = (1 << Pin(BMP_En));	//Ustaw pin En jako wyjœcie
	Port(BMPP_En).DIRSET = (1 << Pin(BMPP_En));	//Ustaw pin drugiego czujnika En jako wyjœcie
	Port(BMPP_En).OUTSET = (1 << Pin(BMPP_En));	//Ustaw pin drugiego czujnika En jako wyjœcie w stanie wysokim
	BMP280_CS(false);
	_delay_us(1);
	BMP280_CS(true);			//Ustaw tryb SPI BMP280, który bêdzie obowi¹zywa³ a¿ do kolejnej utraty zasilania
	BMP280_calibs=malloc(sizeof(BMP280_calibs));		//Przydziel pamiêæ na kalibracjê
	BMP280_readCoefficients();		//Odczytaj parametry kalibracyjne

#ifdef USE_BME280
	BMP280_WriteRegs(BME280_CTRL_HUM, &(uint8_t){0x04}, 1);  //Chwilowo, aby odpaliæ sta³¹ konwersjê wilgotnoœci - oversampling 16x
															 //Ten rejestr musi zostaæ zapisany przed rejestrem BMP280_REGISTER_CONTROL
#endif

	BMP280_WriteRegs(BMP280_REGISTER_CONTROL, &(uint8_t){0xff}, 1);  //Chwilowo, aby odpaliæ sta³¹ konwersjê temp. i ciœnienia
}

void BMP280_soft_reset()
{
	BMP280_CS(false);
#ifdef USE_BME280
	BMP280_WriteRegs(BME280_CTRL_HUM, &(uint8_t){BMP280_SOFT_RESET_CMD}, 1);
#endif
	BMP280_CS(true);
	_delay_ms(2);
}

void BMP280_readCoefficients()
{
	if(BMP280_calibs)
	{
		BMP280_ReadRegs(BMP280_REGISTER_DIG_T1, &BMP280_calibs->dig_T1, sizeof(BMP280_calibs->dig_T1));
		BMP280_ReadRegs(BMP280_REGISTER_DIG_T2, &BMP280_calibs->dig_T2, sizeof(BMP280_calibs->dig_T2));
		BMP280_ReadRegs(BMP280_REGISTER_DIG_T3, &BMP280_calibs->dig_T3, sizeof(BMP280_calibs->dig_T3));
		
		BMP280_ReadRegs(BMP280_REGISTER_DIG_P1, &BMP280_calibs->dig_P1, sizeof(BMP280_calibs->dig_P1));
		BMP280_ReadRegs(BMP280_REGISTER_DIG_P2, &BMP280_calibs->dig_P2, sizeof(BMP280_calibs->dig_P2));
		BMP280_ReadRegs(BMP280_REGISTER_DIG_P3, &BMP280_calibs->dig_P3, sizeof(BMP280_calibs->dig_P3));
		BMP280_ReadRegs(BMP280_REGISTER_DIG_P4, &BMP280_calibs->dig_P4, sizeof(BMP280_calibs->dig_P4));
		BMP280_ReadRegs(BMP280_REGISTER_DIG_P5, &BMP280_calibs->dig_P5, sizeof(BMP280_calibs->dig_P5));
		BMP280_ReadRegs(BMP280_REGISTER_DIG_P6, &BMP280_calibs->dig_P6, sizeof(BMP280_calibs->dig_P6));
		BMP280_ReadRegs(BMP280_REGISTER_DIG_P7, &BMP280_calibs->dig_P7, sizeof(BMP280_calibs->dig_P7));
		BMP280_ReadRegs(BMP280_REGISTER_DIG_P8, &BMP280_calibs->dig_P8, sizeof(BMP280_calibs->dig_P8));
		BMP280_ReadRegs(BMP280_REGISTER_DIG_P9, &BMP280_calibs->dig_P9, sizeof(BMP280_calibs->dig_P9));

#ifdef USE_BME280
		BMP280_ReadRegs(BME280_REGISTER_DIG_H1, &BMP280_calibs->dig_H1, sizeof(BMP280_calibs->dig_H1));
		BMP280_ReadRegs(BME280_REGISTER_DIG_H2, &BMP280_calibs->dig_H2, sizeof(BMP280_calibs->dig_H2));
		BMP280_ReadRegs(BME280_REGISTER_DIG_H3, &BMP280_calibs->dig_H3, sizeof(BMP280_calibs->dig_H3));
		
		uint8_t temp[3];	//Odczytujemy 24 bity roz³o¿one pomiêdzy dwoma rejestrami
		BMP280_ReadRegs(BME280_REGISTER_DIG_H4, temp, 3);
		BMP280_calibs->dig_H4=(int16_t)(int8_t)temp[0] * 16 + (int16_t)(temp[1] & 0x0F);
		BMP280_calibs->dig_H5=(int16_t)(int8_t)temp[2] * 16 + (int16_t)(temp[1] >> 4);
		
		BMP280_ReadRegs(BME280_REGISTER_DIG_H6, &BMP280_calibs->dig_H6, sizeof(BMP280_calibs->dig_H6));
#endif		
	}
}

int8_t BMP280_get_status()
{
	uint8_t status;
	
	BMP280_CS(false);	//Aktywuj uk³ad BMP
	BMP280_ReadRegs(BMP280_STATUS_ADDR, &status, 1);
	BMP280_CS(true);
	return status;
}

uint8_t BMP280_getID()
{
	uint8_t ID;
	
	BMP280_CS(false);	//Aktywuj uk³ad BMP
	BMP280_ReadRegs(BMP280_REGISTER_CHIPID, &ID, 1);
	BMP280_CS(true);
	return ID;	
}

//W czasie odczytu adres rejestru jest autoinkrementowany
void BMP280_ReadRegs(uint8_t reg_addr, void *reg_data, uint8_t len)
{
	BMP280_CS(false);	//Aktywuj uk³ad BMP
	SPI_RW_Byte(reg_addr | BMP280_READ);	//Wyœlij adres rejestru, najstarszy bit sygnalizuje odczyt
	while(len--)
	{
		*(uint8_t*)reg_data=SPI_RW_Byte(0xff);		//Odczytaj bajt danych
		(uint8_t*)reg_data++;
	}
	
	BMP280_CS(true);
}

//W czasie zapisu, zapisywane s¹ pary adres rejestru i dane dla niego
void BMP280_WriteRegs(uint8_t reg_addr, const uint8_t *reg_data, uint8_t len)
{
	reg_addr=reg_addr & 0x7F;	//Skasuuj najstarszy bit, sygnalizuj¹c zapis
	BMP280_CS(false);	//Aktywuj uk³ad BMP
	while(len--)
	{
		SPI_RW_Byte(reg_addr++);
		SPI_RW_Byte(*reg_data);		//Zapisz bajt danych
		reg_data++;
	}
	
	BMP280_CS(true);
}

int32_t BMP280_comp_temp_32bit()
{
	int32_t var1;
	int32_t var2;
	int32_t temperature = 0;
	uint8_t temp[3];	//Bufor na odczytane dane
	BMP280_ReadRegs(BMP280_REGISTER_TEMPDATA, temp, 3);		//Odczytaj dane o temperaturze

	int32_t uncomp_temp = (int32_t) ((((int32_t) (temp[0])) << 12) | (((int32_t) (temp[1])) << 4) | (((int32_t) (temp[2])) >> 4));
	
	var1 = ((((uncomp_temp >> 3) - ((int32_t) BMP280_calibs->dig_T1 << 1))) * ((int32_t) BMP280_calibs->dig_T2)) >> 11;
	var2 = (((((uncomp_temp >> 4) - ((int32_t) BMP280_calibs->dig_T1)) * ((uncomp_temp >> 4) - ((int32_t) BMP280_calibs->dig_T1))) >> 12)
			* ((int32_t) BMP280_calibs->dig_T3)) >> 14;

	BMP280_calibs->t_fine = var1 + var2;
	temperature = (BMP280_calibs->t_fine * 5 + 128) >> 8;
	
	return temperature;
}

uint32_t BMP280_comp_pres_32bit()
{
	int32_t var1;
	int32_t var2;
	uint32_t pressure = 0;
	uint8_t temp[3];	//Bufor na odczytane dane
	BMP280_ReadRegs(BMP280_REGISTER_PRESSUREDATA, temp, 3);		//Odczytaj dane o temperaturze
	uint32_t uncomp_press = (int32_t) ((((uint32_t) (temp[0])) << 12) | (((uint32_t) (temp[1])) << 4) | ((uint32_t) temp[2] >> 4));
	
	var1 = (((int32_t) BMP280_calibs->t_fine) >> 1) - (int32_t) 64000;
	var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int32_t) BMP280_calibs->dig_P6);
	var2 = var2 + ((var1 * ((int32_t) BMP280_calibs->dig_P5)) << 1);
	var2 = (var2 >> 2) + (((int32_t) BMP280_calibs->dig_P4) << 16);
	var1 = (((BMP280_calibs->dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((int32_t) BMP280_calibs->dig_P2) * var1) >> 1)) >> 18;
	var1 = ((((32768 + var1)) * ((int32_t) BMP280_calibs->dig_P1)) >> 15);
	pressure = (((uint32_t) (((int32_t) 1048576) - uncomp_press) - (var2 >> 12))) * 3125;
	
	if (var1 != 0) 
	{
		/* Check for overflows against UINT32_MAX/2; if pres is left-shifted by 1 */
		if (pressure < 0x80000000) pressure = (pressure << 1) / ((uint32_t) var1);
			else pressure = (pressure / (uint32_t) var1) * 2;

		var1 = (((int32_t) BMP280_calibs->dig_P9) * ((int32_t) (((pressure >> 3) * (pressure >> 3)) >> 13))) >> 12;
		var2 = (((int32_t) (pressure >> 2)) * ((int32_t) BMP280_calibs->dig_P8)) >> 13;
		pressure = (uint32_t) ((int32_t) pressure + ((var1 + var2 + BMP280_calibs->dig_P7) >> 4));
	} else 
	{
		pressure = 0;
	}
	
	return pressure;
}

uint32_t BMP280_comp_pres_64bit()
{
	int64_t pressure = 0;
	int64_t var1;
	int64_t var2;

	uint8_t temp[3];	//Bufor na odczytane dane
	BMP280_ReadRegs(BMP280_REGISTER_PRESSUREDATA, temp, 3);		//Odczytaj dane o temperaturze
	uint32_t uncomp_press = (int32_t) ((((uint32_t) (temp[0])) << 12) | (((uint32_t) (temp[1])) << 4) | ((uint32_t) temp[2] >> 4));

	var1 = ((int64_t) (BMP280_calibs->t_fine)) - 128000;
	var2 = var1 * var1 * (int64_t) BMP280_calibs->dig_P6;
	var2 = var2 + ((var1 * (int64_t) BMP280_calibs->dig_P5) << 17);
	var2 = var2 + (((int64_t) BMP280_calibs->dig_P4) << 35);
	var1 = ((var1 * var1 * (int64_t) BMP280_calibs->dig_P3) >> 8)
	+ ((var1 * (int64_t) BMP280_calibs->dig_P2) << 12);
	var1 = ((INT64_C(0x800000000000) + var1) * ((int64_t) BMP280_calibs->dig_P1)) >> 33;
	if (var1 != 0) 
	{
		pressure = 1048576 - uncomp_press;
		pressure = (((pressure << 31) - var2) * 3125) / var1;
		var1 = (((int64_t) BMP280_calibs->dig_P9) * (pressure >> 13) * (pressure >> 13)) >> 25;
		var2 = (((int64_t) BMP280_calibs->dig_P8) * pressure) >> 19;
		pressure = ((pressure + var1 + var2) >> 8) + (((int64_t) BMP280_calibs->dig_P7) << 4);
	} else 
	{
		pressure = 0;
	}

	return (uint32_t) pressure;
}

#ifdef USE_BME280
uint32_t BME280_comp_humid_32bit()
{
	int32_t var1;
	int32_t var2;
	int32_t var3;
	int32_t var4;
	int32_t var5;
	uint32_t humidity;
	uint32_t humidity_max = 102400;
	
	uint8_t temp[2];	//Bufor na odczytane dane
	BMP280_ReadRegs(BME280_REGISTER_HUM_DATA, temp, 2);		//Odczytaj dane o wilgotnoœci
	uint32_t uncomp_hum = ((uint32_t)temp[0] << 8) | temp[1];
	
	var1 = BMP280_calibs->t_fine - ((int32_t)76800);
	var2 = (int32_t)(uncomp_hum * 16384);
	var3 = (int32_t)(((int32_t)BMP280_calibs->dig_H4) * 1048576);
	var4 = ((int32_t)BMP280_calibs->dig_H5) * var1;
	var5 = (((var2 - var3) - var4) + (int32_t)16384) / 32768;
	var2 = (var1 * ((int32_t)BMP280_calibs->dig_H6)) / 1024;
	var3 = (var1 * ((int32_t)BMP280_calibs->dig_H3)) / 2048;
	var4 = ((var2 * (var3 + (int32_t)32768)) / 1024) + (int32_t)2097152;
	var2 = ((var4 * ((int32_t)BMP280_calibs->dig_H2)) + 8192) / 16384;
	var3 = var5 * var2;
	var4 = ((var3 / 32768) * (var3 / 32768)) / 128;
	var5 = var3 - ((var4 * ((int32_t)BMP280_calibs->dig_H1)) / 16);
	var5 = (var5 < 0 ? 0 : var5);
	var5 = (var5 > 419430400 ? 419430400 : var5);
	humidity = (uint32_t)(var5 / 4096);

	if (humidity > humidity_max) humidity = humidity_max;

	return humidity;
}
#endif