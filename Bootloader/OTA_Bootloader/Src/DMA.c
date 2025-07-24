/*
 * DMA.c
 *
 *  Created on: Jun 12, 2025
 *      Author: Echxxanh UET
 */

#include "DMA.h"
#include "UART.h"
#include "Ringbuffer.h"
#include "RCC.h"
#include "Interrupt.h"

uint8_t dma_rx_buffer[DMA_RX_BUFF_SIZE];
uint8_t dma_tx_buffer[DMA_TX_BUFF_SIZE];
volatile uint16_t old_pos = 0;

void DMA1_Channel5_UART1_RX_setup(void)
{
	// DMA rcc = AHB = 8mhz
	RCC->AHBENR |= (1 << 0);
	// Disable DMA1_CN1
	DMA1_Channel5->CCR &= ~DMA_CCR1_EN;
	UART1->CR3 |= UART_CR3_DMAR;
	// GÁn địa chỉ ngoại vi
	DMA1_Channel5->CPAR = (uint32_t)&UART1->DR;
	// Gán địa chỉ đích
	DMA1_Channel5->CMAR = (uint32_t)dma_rx_buffer;
	// buff size
	DMA1_Channel5->CNDTR = DMA_RX_BUFF_SIZE;
	// Cấu hình: tăng sau mỗi byte, vòng, highprio, enable DMA
	DMA1_Channel5->CCR |= DMA_CCR1_MINC | DMA_CCR1_CIRC | DMA_CCR1_PL1_HIGH | DMA_CCR1_EN;
}

void DMA1_Channel5_update_ringbuffer(void)
{
	// Lấy pos hiện tại trong dma rx buffer
	uint16_t pos = DMA_RX_BUFF_SIZE - DMA1_Channel5->CNDTR;
	// Check nếu có data mới gửi tới
	if(pos != old_pos)
	{
		uint16_t i = old_pos;
		while(i != pos)
		{
			Ringbuffer_put(ringbuffer, dma_rx_buffer[i]);
			i = (i + 1) % DMA_RX_BUFF_SIZE;
		}
		old_pos = pos;
	}
}

void DMA1_Channel4_UART1_TX_Interrupt_setup(void)
{
	//RCC->AHBENR |= (1 << 0);
	// Tắt DMA1_CN4
	DMA1_Channel4_UART1_TX_Interrupt_Disable();
	// Enable DMA-RX
	UART1->CR3 |= UART_CR3_DMAT;
	// Hướng: memory to peripheral
	DMA1_Channel4->CCR |= DMA_CCR1_DIR;
	// Enable DMA transfer complete interrupt
	DMA1_Channel4->CCR |= DMA_CCR1_TCIE;
	// GÁn địa chỉ ngoại vi
	DMA1_Channel4->CPAR = (uint32_t)&UART1->DR;
	// Gán địa chỉ đích
	DMA1_Channel4->CMAR = (uint32_t)dma_tx_buffer;
	// buff size
	DMA1_Channel4->CNDTR = DMA_TX_BUFF_SIZE;
	// Cấu hình: tăng sau mỗi byte, highprio
	DMA1_Channel4->CCR |= DMA_CCR1_MINC | DMA_CCR1_PL1_HIGH;
	// Enable NVIC DMA1 channel 4
	NVIC_Enable_IRQ(DMA1_Channel4_IRQ_NUM);
}

void DMA1_Channel4_UART1_TX_Interrupt_Disable(void)
{
	DMA1_Channel4->CCR &= ~DMA_CCR1_EN;
}

void DMA1_Channel4_UART1_TX_Interrupt_Enable(void)
{
	DMA1_Channel4->CCR |= DMA_CCR1_EN;
}
