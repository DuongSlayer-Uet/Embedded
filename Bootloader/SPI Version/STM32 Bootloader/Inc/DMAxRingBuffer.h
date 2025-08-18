/*
 * DMAxRingBuffer.h
 *
 *  Created on: Aug 14, 2025
 *      Author: ACER
 */

#ifndef DMAXRINGBUFFER_H_
#define DMAXRINGBUFFER_H_

/*
 * Brief:	Hàm này dùng để get data from DMA buffer and put to Ringbuffer
 * 			Buffer của DMA cũng có tính chất circular,
 * 			khi đạt max sẽ tự quay về 0.
 * 			Thanh ghi CNDTR sẽ cho biết vị trí hiện tại của DMA buffer đang ghi đến đâu
 */
void DMAxRingBuffer_UpdateData(void);

#endif /* DMAXRINGBUFFER_H_ */
