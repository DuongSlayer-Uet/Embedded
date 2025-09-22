/*
 * Timer.c
 *
 *  Created on: May 26, 2025
 *      Author: EchXanhUET
 */

#include "Timer.h"

void setup_timer1(void)
{
	// Pres = 8, fcpu = HSI = 8mhz, => ftimer = 8000000/8 = 1Mhz
	TIM1->PSC = 7;
	// Count max 0->999
	TIM1->ARR = 999;
	// Enable count
	TIM1->CR1 |= (1 << 0);
}

void delay_ms(uint32_t ms)
{
	for(int i = 0; i < ms; i++)
	{
		TIM1->CNT = 0;
		while((TIM1->SR & (1 << 0)) == 0x00);
		TIM1->SR &= ~(1 << 0);
	}
}

