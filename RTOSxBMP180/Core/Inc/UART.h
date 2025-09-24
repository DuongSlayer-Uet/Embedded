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
}UART1xRS485Typedef;


#define UART1xRS485		((UART1xRS485Typedef*)0x40013800)
// CR1 Register bit
#define UART1xRS485_UE		(1 << 13)
#define UART1xRS485_TE		(1 << 3)
#define UART1xRS485_RE 		(1 << 2)
#define UART1xRS485_RXNEIE 	(1 << 5)
#define UART1xRS485_TXEIE	(1 << 7)
// SR Register bit
#define UART1xRS485_TXE		(1 << 7)
#define UART1xRS485_RXNE	(1 << 5)
#define UART1xRS485_UART_ORE		(1 << 3)
#define	UART1xRS485_FE		(1 << 1)
#define UART1xRS485_PE		(1 << 0)
// CR3 Register bit
#define UART1xRS485_CR3_DMAR	(1 << 6)
#define UART1xRS485_CR3_DMAT	(1 << 7)
/*
 * @brief Cấp clock cho gpioA, UART1xRS485 và config input ouput cho RXTX
 * @param void
 * @retval none
 */
void UART1xRS485_init(void);
/*
 * @brief Gửi dữ liệu
 * @param
 * @retval none
 */
void UART1xRS485_send_data(char data);
/*
 * @brief Nhận dữ liệu
 * @param
 * @retval none
 */
char UART1xRS485_reveive_data(void);
/*
 * @brief Set baud 9600, setup rx interrupt
 * @param ký tự kiểu char
 * @retval none
 */
void UART1xRS485_RX_Int_setup(void);
/*
 * @brief gửi log infor bằng uart while()
 * @param char* string
 */
void UART1xRS485_Log(char str[]);

#endif /* UART_H_ */
