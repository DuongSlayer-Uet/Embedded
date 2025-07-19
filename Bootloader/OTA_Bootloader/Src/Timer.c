/*
 * Timer.c
 *
 *  Created on: May 26, 2025
 *      Author: EchXanhUET
 */

#include "Timer.h"
#include "RCC.h"

void enable_clock_for_timer1(void)
{
	RCC_APB2ENR |= (1 << 11);
}

void setup_timer1(void)
{
	// Pres = 8, fcpu = HSI = 8mhz, => ftimer = 8000000/8 = 1Mhz
	TIM1->PSC = 7;
	// Count max 0->999
	TIM1->ARR = 999;
	// Enable count
	TIM1->CR1 |= (1 << 0);
}

void delay_1s(void)
{
	for(int i = 0; i < 100; i++)
	{
		TIM1->CNT = 0;
		while((TIM1->SR & (1 << 0)) == 0x00);
		TIM1->SR &= ~(1 << 0);
	}
}

