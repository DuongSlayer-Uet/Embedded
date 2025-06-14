/*
 * DMA.h
 *
 *  Created on: Jun 12, 2025
 *      Author: Echxxanh uet
 */

#ifndef DMA_H_
#define DMA_H_

#include <stdint.h>

typedef struct
{
	volatile uint32_t CCR1;
	volatile uint32_t CNDTR1;
	volatile uint32_t CPAR1;
	volatile uint32_t CMAR1;
}DMA1_Channel5_Typedef;

#define DMA1_Channel5 ((DMA1_Channel5_Typedef*)0x40020058ul)
#define DMA_RX_BUFF_SIZE 1024


// CCR1 Register
#define DMA_CCR1_EN		(1 << 0)
#define DMA_CCR1_MINC	(1 << 7)
#define DMA_CCR1_CIRC	(1 << 5)
#define DMA_CCR1_PL1_HIGH	(0b10 << 12)

void DMA1_Channel5_UART1_RX_setup(void);

void DMA1_Channel5_update_ringbuffer(void);

#endif /* DMA_H_ */
