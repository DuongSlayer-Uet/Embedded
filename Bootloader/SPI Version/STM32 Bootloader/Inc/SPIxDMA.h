/*
 * SPIxDMA.h
 *
 *  Created on: Aug 14, 2025
 *      Author: ACER
 */

#ifndef SPIXDMA_H_
#define SPIXDMA_H_

#include <stdint.h>

#define SPIxDMA_RXBUFFSIZE 1024

/*
 * Brief: 	Hàm này Setup DMA cho SPI1, bao gồm DMA RX và DMA TX
 * 			Trong đó DMA RX sử dụng DMA1 channel2
 * 			DMA TX dùng DMA1 channel 3
 * 			DMA TX dùng để spam 1 kí tự respond lại esp, mỗi khi esp request data
 * 			DMA RX dùng để nhận nhanh data từ esp
 */
void SPI1xDMA_SlaveSetup(void);

#endif /* SPIXDMA_H_ */
