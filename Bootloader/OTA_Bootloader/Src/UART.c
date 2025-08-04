/*
 * UART.c
 *
 *  Created on: Jun 3, 2025
 *      Author: EEch xanh UET
 */
#include "UART.h"
#include "gpio.h"
#include "RCC.h"
#include "DMA.h"
#include <string.h>

extern uint8_t dma_tx_buffer[DMA_TX_BUFF_SIZE];

void UART1_init(void)
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
	UART1->BRR = 0x45;

	// Enable TX RX Uart
	UART1->CR1 |= UE | TE | RE;
}

void UART1_RX_Int_setup(void)
{
	// Enable RX Uart
	UART1->CR1 |= RXNEIE;
}

void UART1_send_data(char data)
{
	while(((UART1->SR) & TXE) == 0x00);
	UART1->DR = data;
	while(((UART1->SR) & (1 << 6)) == 0x00);
}

char UART1_reveive_data(void)
{
	while(((UART1->SR) & RXNE) == 0x00);
	return (char)(UART1->DR);
}


void UART_Log(char str[])
{
	int i = 0;
	while(str[i] != '\0')
	{
		UART1_send_data(str[i]);
		i++;
	}
}

void UART_DMA_Log(char str[])
{
	// Tắt dma
	DMA1_Channel4_UART1_TX_Interrupt_Disable();
	uint16_t len = strlen(str);
	memcpy(dma_tx_buffer, str, len);
	// gán lại chuỗi
	DMA1_Channel4->CMAR = (uint32_t)dma_tx_buffer;
	// gán lại len
	DMA1_Channel4->CNDTR = len;
	// Bật dma
	DMA1_Channel4_UART1_TX_Interrupt_Enable();
}
