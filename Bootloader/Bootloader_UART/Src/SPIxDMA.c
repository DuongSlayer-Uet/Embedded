/*
 * SPIxDMA.c
 *
 *  Created on: Aug 14, 2025
 *      Author: ACER
 */

#include "SPI.h"
#include "DMA.h"
#include "RCC.h"
#include "SPIxDMA.h"

uint8_t dummyByte = 0x47;
uint8_t SPIxDMA_RxBuffer[SPIxDMA_RXBUFFSIZE];

void SPI1xDMA_SlaveSetup(void)
{
	RCC->AHBENR |= (1 << 0);

	// ====== DMA RX =======
	// Disable DMA1_CN1
	DMA1_Channel2->CCR &= ~DMA_CCR_EN;
	// SPI1 DMA rx enable
	SPI1->CR2 |= RXDMAEN;
	// GÁn địa chỉ ngoại vi
	DMA1_Channel2->CPAR = (uint32_t)&SPI1->DR;
	// Gán địa chỉ đích
	DMA1_Channel2->CMAR = (uint32_t)SPIxDMA_RxBuffer;
	// buff size
	DMA1_Channel2->CNDTR = SPIxDMA_RXBUFFSIZE;
	// psize, msize
	DMA1_Channel2->CCR &= ~DMA_CCR_PSIZE_MSIZE_8BIT;
	// Dir: Read from peripheral
	DMA1_Channel2->CCR &= ~DMA_CCR_DIR;
	// Cấu hình: tăng sau mỗi byte, vòng, highprio
	DMA1_Channel2->CCR |= DMA_CCR_MINC | DMA_CCR_CIRC | DMA_CCR_PL1_HIGH;

	// ====== DMA TX =======
	// Disable DMA1 channel3
	DMA1_Channel3->CCR &= ~DMA_CCR_EN;
	// Enable SPI TX DMA
	SPI1->CR2 |= TXDMAEN;
	// Source address
	DMA1_Channel3->CPAR = (uint32_t)&SPI1->DR;
	// Target address
	DMA1_Channel3->CMAR = (uint32_t)&dummyByte;
	// Nạp sẵn dummy byte
	SPI1->DR = dummyByte;
	// buff size = 1
	DMA1_Channel3->CNDTR = 1;
	// psize, msize
	DMA1_Channel3->CCR &= ~DMA_CCR_PSIZE_MSIZE_8BIT;
	// Circular, highprio
	DMA1_Channel3->CCR |= DMA_CCR_CIRC | DMA_CCR_PL1_HIGH;
	// Directory: Read from memory
	DMA1_Channel3->CCR |= DMA_CCR_DIR;

	// ENABLE DMA1 channel 2,3
	DMA1_Channel2->CCR |= DMA_CCR_EN;
	DMA1_Channel3->CCR |= DMA_CCR_EN;
}
