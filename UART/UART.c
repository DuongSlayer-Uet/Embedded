/*
 * UART.c
 *
 *  Created on: Jun 3, 2025
 *      Author: EEch xanh UET
 */
#include "UART.h"
#include "RCC.h"

void UART1_gpio_init(void)
{
	// Clock Init
	RCC->APB2ENR |= (1 << 2);	// GPIOA
	RCC->APB2ENR |= (1 << 14);	// USART1

	// GPIOA9 - TX - Output max 10mhz, AF push pull
	GPIOA->CRH &= ~(0b1111 << 4);
	GPIOA->CRH |= (0b1001 << 4);

	// GPIOA10 - RX - Input, floating
	GPIOA->CRH &= ~(0b1111 << 8);
	GPIOA->CRH |= (0b0100 << 8);
}

// BRR = fcpu/(16*baud)
void UART1_baud_init(void)
{
	// Baud 9600
	UART1->BRR = (52 << 4) | 1;

	// Enable TX RX Uart
	UART1->CR1 |= UE | TE | RE;
}

void UART1_send_data(char data)
{
	while(((UART1->SR) & TXE) == 0x00);
	UART1->DR = data;
	while(((UART1->SR) & (1 << 6)) == 0x00);
}

