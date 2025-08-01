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
	volatile uint32_t ISR;
	volatile uint32_t IFCR;
}DMA_INT_Typedef;

typedef struct
{
	volatile uint32_t CCR;
	volatile uint32_t CNDTR;
	volatile uint32_t CPAR;
	volatile uint32_t CMAR;
}DMA_Channel_Typedef;


#define DMA1_Channel5 			((DMA_Channel_Typedef*)0x40020058ul)
#define DMA1_Channel4 			((DMA_Channel_Typedef*)0x40020044ul)
#define DMA1					((DMA_INT_Typedef*)0x40020000ul)
#define DMA_RX_BUFF_SIZE 1024
#define DMA_TX_BUFF_SIZE 1024


// CCR1 Register
#define DMA_CCR1_EN		(1 << 0)
#define DMA_CCR1_MINC	(1 << 7)
#define DMA_CCR1_CIRC	(1 << 5)
#define DMA_CCR1_PL1_HIGH	(0b10 << 12)
#define DMA_CCR1_DIR	(1 << 4)
#define DMA_CCR1_TCIE	(1 << 1)

void DMA1_Channel5_UART1_RX_setup(void);

void DMA1_Channel5_update_ringbuffer(void);

void DMA1_Channel5_UART1_TX_Interrupt_setup(void);

void DMA1_Channel4_UART1_TX_Interrupt_setup(void);

void DMA1_Channel4_UART1_TX_Interrupt_Disable(void);

void DMA1_Channel4_UART1_TX_Interrupt_Enable(void);

#endif /* DMA_H_ */
