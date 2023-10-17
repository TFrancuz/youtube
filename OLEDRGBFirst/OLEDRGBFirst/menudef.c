/*
 * menudef.c
 *
 * Created: 2013-08-20 13:05:36
 *  Author: tmf
 */

#include <Menu\menu.h>
#include <util/delay.h>
#include <OLED\OLED.h>
#include <avr\pgmspace.h>

//Prototypy funkcji obs³ugi wybranej pozycji menu
void menufunc1();
void menufunc2();
void menufunc3();
void menufunc4();
void menufunc5();
void menufunc6();
void menufunc7();

#define PGM_STR(X) ((const char[]) { X })

struct _menuitem const __flash menu;
struct _menuitem const __flash menuA1;
struct _menuitem const __flash menuB1;

struct _menuitem const __flash menuB3 = {PGM_STR("<z powrotem>"), Menu_Back, &menuB1, 0, 0};
struct _menuitem const __flash menuB2 = {PGM_STR("Podmenu B2"), menufunc7, &menuB1, 0, &menuB3};
struct _menuitem const __flash menuB1 = {PGM_STR("Podmenu B1"), menufunc6, &menuA1, 0, &menuB2};

struct _menuitem const __flash menuA4 = {PGM_STR("<z powrotem>"), Menu_Back, &menuA1, 0, 0};
struct _menuitem const __flash menuA3 = {PGM_STR("Podmenu A3"), menufunc5, &menuA1, 0, &menuA4};
struct _menuitem const __flash menuA2 = {PGM_STR("Podmenu A2"), 0, &menuA1, &menuB1, &menuA3};
struct _menuitem const __flash menuA1 = {PGM_STR("Podmenu A1"), menufunc4, &menu, 0, &menuA2};

struct _menuitem const __flash menu4 = {PGM_STR("Menu5"), 0, &menu, 0, 0};
struct _menuitem const __flash menu3 = {PGM_STR("Menu4"), menufunc3, &menu, 0, &menu4};
struct _menuitem const __flash menu2 = {PGM_STR("Menu3"), menufunc2, &menu, 0, &menu3};
struct _menuitem const __flash menu1 = {PGM_STR("Menu2"), 0, &menu, &menuA1, &menu2};

const __flash struct _menuitem const __flash menu = {PGM_STR("Menu1"), menufunc1, 0, 0, &menu1};

extern const __flash uint8_t* const __flash system16_array[];
extern const __flash uint8_t* const __flash system12_array[];
extern const __flash uint8_t* const __flash system8_array[];

const __flash uint8_t* const __flash *Menu_font = system8_array;            //Czcionka u¿ywana do wyœwietlenia menu
//const __flash uint8_t* const __flash *Menu_font = system12_array;            //Czcionka u¿ywana do wyœwietlenia menu


void menufunc1()
{
	OLED_Clear(false); //Wyczyœæ LCD
	OLED_SetTextOp(0, 0, "Menu 1!", system8_array, false);
	_delay_ms(2000);
}

void menufunc2()
{
	OLED_Clear(false); //Wyczyœæ LCD
	OLED_SetTextOp(0, 0, "Menu 3!", system8_array, false);
	_delay_ms(2000);
}

void menufunc3()
{
	OLED_Clear(false); //Wyczyœæ LCD
	OLED_SetTextOp(0, 0, "Menu 4!", system8_array, false);
	_delay_ms(2000);
}

void menufunc4()
{
	OLED_Clear(false); //Wyczyœæ LCD
	OLED_SetTextOp(0, 0, "Podmenu A1!", system8_array, false);
	_delay_ms(2000);
}

void menufunc5()
{
	OLED_Clear(false); //Wyczyœæ LCD
	OLED_SetTextOp(0, 0, "Podmenu A3!", system8_array, false);
	_delay_ms(2000);
}

void menufunc6()
{
	OLED_Clear(false); //Wyczyœæ LCD
	OLED_SetTextOp(0, 0, "Podmenu B1!", system8_array, false);
	_delay_ms(2000);
}

void menufunc7()
{
	OLED_Clear(false); //Wyczyœæ LCD
	OLED_SetTextOp(0, 0, "Podmenu B2!", system8_array, false);
	_delay_ms(2000);
}

