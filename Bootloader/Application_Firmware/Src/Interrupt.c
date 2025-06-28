/*
 * Interrupt.c
 *
 *  Created on: May 24, 2025
 *      Author: ÉchxanhUet :>
 */

#include "Interrupt.h"
#include "UART.h"
#include "gpio.h"
#include "flash.h"
#include "Ringbuffer.h"
#include <stdint.h>

extern volatile uint8_t goToBootloader;

void NVIC_Enable_IRQ(uint8_t EXTI_IRQ_NUM)
{
	if(EXTI_IRQ_NUM < 32)
	{
		NVIC_ISER0 |= (1 << EXTI_IRQ_NUM);
	}
	else if(EXTI_IRQ_NUM < 64)
	{
		NVIC_ISER1 |= (1 << (EXTI_IRQ_NUM - 32));
	}
}


void EXTI_Init(GPIO_Typedef* gpio, uint8_t pin, uint8_t edge_type)
{
	if(pin < 8)
	{
		// Clear bit
		gpio->CRL &= ~(0xF << (pin*4));
		// Input mode, Pull up/down: 1000
		gpio->CRL |= (0b1000 << (pin*4));
	}
	else if(pin < 16)
	{
		// Clear bit
		gpio->CRH &= ~(0xF << ((pin - 8)*4));
		// Input mode, Pull up/down: 1000
		gpio->CRH |= (0b1000 << ((pin - 8)*4));
	}
	// Pull up
	gpio->ODR &= ~(1 << pin);
	gpio->ODR |= (1 << pin);
	// Enable Interrupt linee
	uint8_t port_code = 0;
	if(gpio == GPIOA) port_code = 0;
	else if(gpio == GPIOB) port_code = 1;
	else if(gpio == GPIOC) port_code = 2;
	else if(gpio == GPIOD) port_code = 3;

	if(pin < 4)
	{
		AFIO->EXTICR1 &= ~(0xF << pin*4);
		AFIO->EXTICR1 |= (port_code << pin*4);
	}
	else if(pin < 8)
	{
		AFIO->EXTICR2 &= ~(0xF << (pin - 4)*4);
		AFIO->EXTICR2 |= (port_code << (pin - 4)*4);
	}
	else if(pin < 12)
	{
		AFIO->EXTICR3 &= ~(0xF << (pin - 8)*4);
		AFIO->EXTICR3 |= (port_code << (pin - 8)*4);
	}
	else if(pin < 16)
	{
		AFIO->EXTICR4 &= ~(0xF << (pin - 12)*4);
		AFIO->EXTICR4 |= (port_code << (pin - 12)*4);
	}
	// Unable mask at line 0
	EXTI->IMR |= (1 << pin);
	// Enable Falling Edge Detection
	if(edge_type == 0)
	{
		EXTI->RTSR &= ~(1 << pin);
		EXTI->FTSR |= (1 << pin);
	}
	else if(edge_type == 1)
	{
		EXTI->FTSR &= ~(1 << pin);
		EXTI->RTSR |= (1 << pin);
	}
}

void USART1_RX_Int_init(void)
{
	UART1_RX_Int_setup();
	NVIC_Enable_IRQ(USART1);
}

void USART1_IRQHandler(void)
{
	if(UART1->SR & RXNE)
	{
		// Check các bit mask, đảm bảo không phải tín hiệu nhiễu
		if((UART1->SR & (UART_ORE | UART_PE | UART_FE)) == 0)
		{
			char data = (char)(UART1->DR);
			if(data == 'A')
			{
				goToBootloader = 1;
			}
		}
	}
}

void EXTI0_IRQHandler(void)
{
	if(EXTI->PR & (1 << 0))
	{
		// Clear interrupt flag
		EXTI->PR |= (1 << 0);
		// Do something
		//GPIO_toggle_pin(GPIOC, 13);

	}
}

void EXTI1_IRQHandler(void)
{
	if(EXTI->PR & (1 << 1))
	{
		// Clear interrupt flag
		EXTI->PR |= (1 << 1);
		// Do something
		//GPIO_toggle_pin(GPIOC, 13);

	}
}

void EXTI2_IRQHandler(void)
{
	if(EXTI->PR & (1 << 2))
	{
		// Clear interrupt flag
		EXTI->PR |= (1 << 2);

		// Do something
		//GPIO_toggle_pin(GPIOC, 13);

	}
}

void EXTI3_IRQHandler(void)
{
	if(EXTI->PR & (1 << 3))
	{
		// Clear interrupt flag
		EXTI->PR |= (1 << 3);
		// Do something
		//GPIO_toggle_pin(GPIOC, 13);

	}
}

void EXTI4_IRQHandler(void)
{
	if(EXTI->PR & (1 << 4))
	{
		// Clear interrupt flag
		EXTI->PR |= (1 << 4);
		// Clear flag ở page 50
		Flash_EraseOnePage(0x0800C800);
		// Flag = 0x0ABC
		Flash_WriteHalfWord(0x0800C800, 0x0ABC);
		// Toggle báo hiệu
		GPIO_toggle_pin(GPIOC, 13);
		// Soft reset, 0x05FA là key, bit 2 là bit reset
		RESET_AIRCR = (0x05FA << 16) | (1 << 2);
	}
}

void EXTI9_5_IRQHandler(void)
{
	for(int i = 5; i < 10; i++)
	{
		if(EXTI->PR & (1 << i))
		{
			// CLEAR FLAG
			EXTI->PR |= (1 << i);
		}
	}
}

void EXTI15_10_IRQHandler(void)
{
	for(int i = 10; i < 16; i++)
	{
		if(EXTI->PR & (1 << i))
		{
			// CLEAR FLAG
			EXTI->PR |= (1 << i);
			GPIO_toggle_pin(GPIOC, 13);
		}
	}
}
