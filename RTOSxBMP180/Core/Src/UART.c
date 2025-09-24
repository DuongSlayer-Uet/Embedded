/*
 * UART.c
 *
 *  Created on: Jun 3, 2025
 *      Author: EEch xanh UET
 */
#include "UART.h"
#include "gpio.h"
#include "RCC.h"
#include <string.h>

void UART1xRS485_init(void)
{
	// Clock Init
	RCC->APB2ENR |= (1 << 2);	// GPIOA
	RCC->APB2ENR |= (1 << 14);	// USART1

	// GPIOA9 - TX - Output max 10mhz, AF push pull
	GPIOA->CRH &= ~(0b1111 << 4);
	GPIOA->CRH |= (0b1001 << 4);

	// GPIOA10 - RX - Input, pullup
	GPIOA->CRH &= ~(0b1111 << 8);
	GPIOA->CRH |= (0b1000 << 8);
	GPIOA->ODR |= (1 << 10);

	// Baud 9600
	UART1xRS485->BRR = (52 << 4) | 1;

	// Enable TX RX Uart
	UART1xRS485->CR1 |= UART1xRS485_UE | UART1xRS485_TE | UART1xRS485_RE;
}

void UART1xRS485_RX_Int_setup(void)
{
	// Enable RX Uart
	UART1xRS485->CR1 |= UART1xRS485_RXNEIE;
}

void UART1xRS485_send_data(char data)
{
	while(((UART1xRS485->SR) & UART1xRS485_TXE) == 0x00);
	UART1xRS485->DR = data;
	while(((UART1xRS485->SR) & (1 << 6)) == 0x00);
}

char UART1xRS485_reveive_data(void)
{
	while(((UART1xRS485->SR) & UART1xRS485_RXNE) == 0x00);
	return (char)(UART1xRS485->DR);
}


void UART1xRS485_Log(char str[])
{
	int i = 0;
	while(str[i] != '\0')
	{
		UART1xRS485_send_data(str[i]);
		i++;
	}
}
