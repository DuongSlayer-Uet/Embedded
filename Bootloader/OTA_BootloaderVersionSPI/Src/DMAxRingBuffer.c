/*
 * DMAxRingBuffer.c
 *
 *  Created on: Aug 14, 2025
 *      Author: ACER
 */

#include "DMAxRingBuffer.h"
#include "Ringbuffer.h"
#include "SPIxDMA.h"
#include "DMA.h"
#include <stdint.h>

extern uint8_t SPIxDMA_RxBuffer[SPIxDMA_RXBUFFSIZE];
extern RingBuffer_Typedef RingBuffer;
volatile uint16_t old_pos = 0;

void DMAxRingBuffer_UpdateData(void)
{
	// Lấy pos hiện tại trong dma rx buffer
	uint16_t pos = SPIxDMA_RXBUFFSIZE - DMA1_Channel2->CNDTR;
	// Check nếu có data mới gửi tới
	if(pos != old_pos)
	{
		uint16_t i = old_pos;
		while(i != pos)
		{
			Ringbuffer_put(&RingBuffer, SPIxDMA_RxBuffer[i]);
			i = (i + 1) % SPIxDMA_RXBUFFSIZE;
		}
		old_pos = pos;
	}
}
