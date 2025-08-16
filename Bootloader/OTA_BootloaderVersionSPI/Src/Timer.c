/*
 * Timer.c
 *
 *  Created on: May 26, 2025
 *      Author: EchXanhUET
 */

#include "Timer.h"
#include "RCC.h"

void TIM1_SetUp(void)
{
	// 8mhz
	RCC_APB2ENR |= (1 << 11);
	// Pres = 8, fcpu = HSI = 8mhz, => ftimer = 8000000/8 = 1Mhz
	TIM1->PSC = 7;
	// Count max 0->999
	TIM1->ARR = 999;
}

void TIM1_Delay1s(void)
{
	TIM1->CR1 |= (1 << 0);
	for(int i = 0; i < 1000; i++)
	{
		TIM1->CNT = 0;
		while((TIM1->SR & (1 << 0)) == 0x00);
		TIM1->SR &= ~(1 << 0);
	}
	TIM1->CR1 &= ~(1 << 0);
}

void TIM1_DelayMs(uint32_t ms)
{
	TIM1->CR1 |= (1 << 0);
	for(int i = 0; i < ms; i++)
	{
		TIM1->CNT = 0;
		while((TIM1->SR & (1 << 0)) == 0x00);
		TIM1->SR &= ~(1 << 0);
	}
	TIM1->CR1 &= ~(1 << 0);
}

void TIM1_StartCounting(void)
{
	TIM1->CNT = 0;				// Reset counter
	TIM1->SR &= ~(1 << 0);		// Reset ovf flag
	TIM1->DIER |= (1 << 0);		// Turn on ovf interrupt
	TIM1->CR1 |= (1 << 0);		// Counter enable
}

uint32_t TIM1_GetCNT(void)
{
    TIM1->DIER &= ~(1 << 0);        // Disable update interrupt
    TIM1->CR1 &= ~(1 << 0);         // Stop counter
    return TIM1->CNT;               // Return current CNT
}
