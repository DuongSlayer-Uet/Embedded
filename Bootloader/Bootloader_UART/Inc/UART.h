/*
 * UART.h
 *
 *  Created on: Jun 3, 2025
 *      Author: Ecchs xanh UET
 */

#ifndef UART_H_
#define UART_H_

#include <stdint.h>


// Struct UART
typedef struct
{
	volatile uint32_t SR;
	volatile uint32_t DR;
	volatile uint32_t BRR;
	volatile uint32_t CR1;
	volatile uint32_t CR2;
	volatile uint32_t CR3;
}UART_Typedef;


#define UART1		((UART_Typedef*)0x40013800)
// CR1 Register bit
#define UE		(1 << 13)
#define TE		(1 << 3)
#define RE 		(1 << 2)
#define RXNEIE 	(1 << 5)
// SR Register bit
#define TXE		(1 << 7)
#define TXEIE	(1 << 7)
#define RXNE	(1 << 5)
#define UART_ORE		(1 << 3)
#define	UART_FE		(1 << 1)
#define UART_PE		(1 << 0)
/*
 * @brief Cấp clock cho gpioA, uart1 và config input ouput cho RXTX
 * @param void
 * @retval none
 */
void UART1_init(void);
/*
 * @brief Gửi dữ liệu
 * @param
 * @retval none
 */
void UART1_send_data(char data);
/*
 * @brief Nhận dữ liệu
 * @param
 * @retval none
 */
char UART1_reveive_data(void);
/*
 * @brief Set baud 9600, setup rx interrupt
 * @param ký tự kiểu char
 * @retval none
 */
void UART1_RX_Int_setup(void);
/*
 * @brief gửi log infor bằng uart while()
 * @param char* string
 */
void UART_Log(char str[]);
/*
 * @brief gửi log bằng dma tx
 */
void UART_DMA_Log(char str[]);

#endif /* UART_H_ */
