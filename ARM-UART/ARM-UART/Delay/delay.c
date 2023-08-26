/*
 * delay.c
 *
 * Created: 2017-01-03 10:54:55
 *  Author: tmf
 */ 

#include "delay.h"

__IO uint32_t TimingDelay;

void TimingDelay_Decrement(void)
{
	if (TimingDelay != 0x00) TimingDelay--;
}

void SysTick_Handler(void)
{
	TimingDelay_Decrement();
}