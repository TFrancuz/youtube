/*
 * delay.h
 *
 * Created: 2017-01-03 10:53:15
 *  Author: tmf
 */ 


#ifndef DELAY_H_
#define DELAY_H_

#include <sam.h>
#include <stdint.h>

extern __IO uint32_t TimingDelay;

static inline void _delay_ms(__IO uint32_t nTime)
{
	TimingDelay = nTime;
	
	while(TimingDelay != 0);
}

#endif /* DELAY_H_ */