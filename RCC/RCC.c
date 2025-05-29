/*
 * RCC.c
 *
 *  Created on: May 27, 2025
 *      Author: ACER
 */
#include "RCC.h"

void RCC_Config_HSE_PLL_48MHz(void)
{
	// FLASH CONFIG
	FLASH_ACR |= PRFTBE;
	FLASH_ACR &= ~(0b111 << 0);
	FLASH_ACR |= LATENCY_TWO;

	// CLOCK CONFIG
	// ENABLE HSE
	RCC->CR |= HSEON;
	while(!(RCC->CR & HSERDY));
	// PLL source = HSE
	RCC->CFGR |= PLLSRC;
	// CONFIG PLL MUL = 6
	RCC->CFGR &= ~(0b1111 << 18);		// CLEAR 4 BIT MUL
	RCC->CFGR |= PLLX6;
	// AHB CLOCK = x? SYSCLK
	RCC->CFGR &= ~(0xF << 4);
	// APB2 = AHB = 48
	RCC->CFGR &= ~(0xF << 11);
	// APB1 = 1/2 AHB = 24
	RCC->CFGR &= ~(0b111 << 8);
	RCC->CFGR |= (1 << 10);

	// Source clock as PLL
	RCC->CFGR &= ~(0b11 << 0);
	RCC->CFGR |= SW_PLL;

	// ENABLE PLL
	RCC->CR |= PLLON;
	while(!(RCC->CR & PLLRDY));
}

void RCC_Config_HSE_PLL_72MHz(void)
{
	// FLASH CONFIG
	FLASH_ACR |= PRFTBE;
	FLASH_ACR &= ~(0b111 << 0);
	FLASH_ACR |= LATENCY_TWO;

	// CLOCK CONFIG
	// ENABLE HSE
	RCC->CR |= HSEON;
	while(!(RCC->CR & HSERDY));

	// PLL source = HSE
	RCC->CFGR |= PLLSRC;

	// CONFIG PLL MUL = 9 =>PLL = 72
	RCC->CFGR &= ~(0b01111 << 18);		// CLEAR 4 BIT MUL
	RCC->CFGR |= PLLX9;

	// AHB CLOCK = SYSCLK
	RCC->CFGR &= ~(0xF << 4);

	// APB1 = 1/2 AHB = 36
	RCC->CFGR &= ~(0b111 << 8);
	RCC->CFGR |= (1 << 10);

	// APB2 = AHB = 72
	RCC->CFGR &= ~(0xF << 11);

	// Source clock as PLL
	RCC->CFGR &= ~(0b11 << 0);
	RCC->CFGR |= SW_PLL;

	// ENABLE PLL
	RCC->CR |= PLLON;
	while(!(RCC->CR & PLLRDY));
}

void RCC_Config_HSI_8MHz(void)
{
	// FLASH CONFIG
	FLASH_ACR |= PRFTBE;
	FLASH_ACR &= ~(0b111 << 0);
	FLASH_ACR |= LATENCY_ZERO;

	// CLOCK CONFIG
	// ENABLE HSI
	RCC->CR |= HSION;
	while(!(RCC->CR & HSIRDY));

	// AHB CLOCK = SYSCLK
	RCC->CFGR &= ~(0xF << 4);

	// APB1 = AHB = 8
	RCC->CFGR &= ~(0b111 << 8);

	// APB2 = AHB = 8
	RCC->CFGR &= ~(0xF << 11);

	// Source clock as HSE
	RCC->CFGR &= ~(0b11 << 0);
	RCC->CFGR |= SW_HSE;
}

void RCC_Config_HSI_PLL_64MHz(void)
{
	// FLASH CONFIG
	FLASH_ACR |= PRFTBE;
	FLASH_ACR &= ~LATENCY_ZERO;
	FLASH_ACR |= LATENCY_TWO;

	// CLOCK CONFIG
	// ENABLE HSI
	RCC->CR |= HSION;
	while(!(RCC->CR & HSIRDY));

	// PLL source = HSI/2 = 4
	RCC->CFGR &= PLLSRC_HSI;

	// CONFIG PLL MUL = 16 =>PLL = 64
	RCC->CFGR &= ~(0b1111 << 18);		// CLEAR 4 BIT MUL
	RCC->CFGR |= PLLX16;

	// AHB CLOCK = SYSCLK
	RCC->CFGR &= ~(0xF << 4);

	// APB1 = 1/2 AHB = 36
	RCC->CFGR &= ~(0b111 << 8);
	RCC->CFGR |= (1 << 10);

	// APB2 = AHB = 72
	RCC->CFGR &= ~(0xF << 11);

	// Source clock as PLL
	RCC->CFGR &= ~(0b11 << 0);
	RCC->CFGR |= SW_PLL;

	// ENABLE PLL
	RCC->CR |= PLLON;
	while(!(RCC->CR & PLLRDY));
}

void RCC_Config(ClockConfig_t clock)
{
	switch(clock)
	{
		case CLOCK_HSI_8MHZ:
			RCC_Config_HSI_8MHz();
			break;
		case CLOCK_HSI_64MHZ:
			RCC_Config_HSI_PLL_64MHz();
			break;
		case CLOCK_HSE_48MHZ:
			RCC_Config_HSE_PLL_48MHz();
			break;
		case CLOCK_HSE_72MHZ:
			RCC_Config_HSE_PLL_72MHz();
			break;
	}
}
