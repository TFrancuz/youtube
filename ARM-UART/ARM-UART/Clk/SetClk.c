/*
 * SetClk.c
 *
 * Created: 2017-01-03 10:49:40
 *  Author: tmf
 */ 

#include "sam.h"
#include "SetClk.h"

void init_32KXTAL()
{
	SYSCTRL->XOSC32K.reg = SYSCTRL_XOSC32K_ENABLE;	//Odblokuj zegar 32768Hz - kwarc
	SYSCTRL->XOSC32K.reg|= SYSCTRL_XOSC32K_STARTUP(5) | SYSCTRL_XOSC32K_AAMPEN | SYSCTRL_XOSC32K_EN32K | SYSCTRL_XOSC32K_XTALEN | SYSCTRL_XOSC32K_ENABLE ;
	
	while((REG_SYSCTRL_PCLKSR & (SYSCTRL_PCLKSR_XOSC32KRDY)) == 0); // zaczekaj a¿ zegar bêdzie gotowy
}

void init_generic_clocks()
{
	GCLK->GENDIV.reg = GCLK_GENDIV_DIV(1) | GCLK_GENDIV_ID(1);  //Brak podzia³u CLK
	GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_XOSC32K | GCLK_GENCTRL_ID(1);  //Generator 1 taktowany z zegara 32768 Hz
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(1) | GCLK_CLKCTRL_ID_DFLL48;  //Uruchom DFLL 48 MHz
}

void init_FDLL48()
{
	SYSCTRL->DFLLCTRL.reg = SYSCTRL_DFLLCTRL_ENABLE ;  //Uruchom DFLL
	SYSCTRL->DFLLMUL.reg = (SYSCTRL_DFLLMUL_FSTEP(10)) | (SYSCTRL_DFLLMUL_CSTEP(5)) | (SYSCTRL_DFLLMUL_MUL(1465));
	SYSCTRL->DFLLCTRL.reg |=  SYSCTRL_DFLLCTRL_MODE;
	
	while((REG_SYSCTRL_PCLKSR & (SYSCTRL_PCLKSR_DFLLLCKC)) == 0);  //Poczekaj na stabilizacjê pêtli
	
	NVMCTRL->CTRLB.reg |= (NVMCTRL_CTRLB_RWS(1));  //Liczba WSów do FLASH
}

void init_generick_clock0()
{
	GCLK->GENDIV.reg = GCLK_GENDIV_DIV(1) | GCLK_GENDIV_ID(0);
	GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_DFLL48M | GCLK_GENCTRL_ID(0);
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_GEN(0) | GCLK_CLKCTRL_CLKEN;
}

void Set48MHzClk()
{
	init_32KXTAL();
	init_generic_clocks();
	init_FDLL48();
	init_generick_clock0();
	SystemCoreClock=48000000UL;  //Bie¿¹ce taktowanie MCU
	SysTick_Config(SystemCoreClock / 1000);
}