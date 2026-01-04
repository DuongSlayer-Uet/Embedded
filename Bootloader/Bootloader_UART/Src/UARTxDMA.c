/*
 * UARTxDMA.c
 *
 *  Created on: Aug 19, 2025
 *      Author: ACER
 */

#include "UARTxDMA.h"
#include "UART.h"
#include "DMA.h"
#include "RCC.h"


uint8_t UARTxDMA_RxBuffer[UARTxDMA_RxBufferSize];

void UARTxDMA_Config(void)
{
	// dma1 clock en
	RCC->AHBENR |= (1 << 0);
	UART1->CR3 |= UART_DMAR;
	// Disable DMA1_CN5
	DMA1_Channel5->CCR &= ~DMA_CCR_EN;
	// GÁn địa chỉ ngoại vi
	DMA1_Channel5->CPAR = (uint32_t)&UART1->DR;
	// Gán địa chỉ đích
	DMA1_Channel5->CMAR = (uint32_t)UARTxDMA_RxBuffer;
	// buff size
	DMA1_Channel5->CNDTR = UARTxDMA_RxBufferSize;
	// Cấu hình: tăng sau mỗi byte, vòng, highprio, enable DMA
	DMA1_Channel5->CCR |= DMA_CCR_MINC | DMA_CCR_CIRC | DMA_CCR_PL1_HIGH | DMA_CCR_EN;
}
