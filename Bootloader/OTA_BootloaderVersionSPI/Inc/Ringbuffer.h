/*
 * Ringbuffer.h
 *
 *  Created on: Jun 11, 2025
 *      Author: ACER
 */

#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

#include <stdint.h>

#define RX_BUFFERSIZE 1024

// Struct Ringbuffer
typedef struct
{
	volatile uint8_t buffer[RX_BUFFERSIZE];
	volatile uint16_t tail;
	volatile uint16_t head;
}RingBuffer_Typedef;

void Ringbuffer_init(RingBuffer_Typedef* ring);

void Ringbuffer_put(RingBuffer_Typedef* ring, uint8_t data);

int Ringbuffer_get(RingBuffer_Typedef* ring, uint8_t* data);

#endif /* RINGBUFFER_H_ */
