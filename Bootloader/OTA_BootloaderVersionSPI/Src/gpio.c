/*
 * gpio.c
 *
 *  Created on: May 23, 2025
 *      Author: Ech xanh UET
 */

#include "gpio.h"
#include "RCC.h"

void GPIO_init_output(GPIO_Typedef* gpio, uint8_t pin)
{
	// Set clock for GPIO_X
	if(gpio == GPIOA)
	{
		(RCC_APB2ENR) |= (1 << 2);
	}
	else if(gpio == GPIOB)
	{
		(RCC_APB2ENR) |= (1 << 3);
	}
	else if(gpio == GPIOC)
	{
		(RCC_APB2ENR) |= (1 << 4);
	}
	else if(gpio == GPIOD)
	{
		(RCC_APB2ENR) |= (1 << 5);
	}


	// output max speed 2mHz
	if(pin < 8)
	{
		gpio->CRL &= ~(0xF << (pin*4));		// Reset MODE and CNF bit to 0
		gpio->CRL |= (0x2 << (pin*4));			// Set MODE and CNF bit to 0010
	}
	else if(pin >= 8)
	{
		pin -= 8;
		gpio->CRH &= ~(0xF << (pin*4));
		gpio->CRH |= (0x2 << (pin*4));
	}
}

void GPIO_set(GPIO_Typedef* gpio, uint8_t pin)
{
	gpio->BSRR = (1 << pin);
}

void GPIO_reset(GPIO_Typedef* gpio, uint8_t pin)
{
	gpio->BSRR = (1 << (pin + 16));
}

void GPIO_toggle_pin(GPIO_Typedef* gpio, uint8_t pin)
{
	if(gpio->ODR & (1 << pin))
	{
		gpio->BSRR = (1 << (pin + 16));
	}
	else
	{
		gpio->BSRR = (1 << pin);
	}
}

void GPIO_InitBootPin(void)
{
	// Clock enable for PB port
	RCC_APB2ENR |= (1 << 3);
	// Input, pull up/pulldown PB12
	GPIOB->CRH &= ~(0xF << 16);
	GPIOB->CRH |= (0b1000 << 16);
	// pull up
	GPIOB->ODR |= (1 << 12);
}
uint32_t GPIO_ReadBootPin(void)
{
	return (GPIOB->IDR & (1 << 12));
}
void GPIO_DisableBootPin(void)
{
	RCC_APB2ENR	&= ~(1 << 3);
}
