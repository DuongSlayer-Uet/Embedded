/*
 * Ringbuffer.c
 *
 *  Created on: Jun 11, 2025
 *      Author: ACER
 */

#include "Ringbuffer.h"

RingBuffer_Typedef ringbuffer_instance;
RingBuffer_Typedef* ringbuffer = &ringbuffer_instance;

void Ringbuffer_put(RingBuffer_Typedef* ring, uint8_t data)
{
	uint16_t next = (ring->head + 1) % RX_BUFFERSIZE;	// nếu đạt max thì reset về 0
	if(next != ring->tail)
	{
		ring->buffer[ring->head] = data;
		ring->head = next;
	}
}

int Ringbuffer_get(RingBuffer_Typedef* ring, uint8_t* data)
{
	if(ring->head == ring->tail) return 0;	// check buffer có dữ liệu hay không
	*data = ring->buffer[ring->tail];
	ring->tail = (ring->tail + 1) % RX_BUFFERSIZE;
	return 1;
}

void Ringbuffer_init(RingBuffer_Typedef* ring)
{
	ring->head = 0;
	ring->tail = 0;
}
