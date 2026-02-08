/*
 * UARTxDMA.h
 *
 *  Created on: Aug 19, 2025
 *      Author: ACER
 */

#ifndef UARTXDMA_H_
#define UARTXDMA_H_

#define UARTxDMA_RxBufferSize	2048
#define UART_DMAR		(1 << 6)

void UARTxDMA_Config(void);

#endif /* UARTXDMA_H_ */
