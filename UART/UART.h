/*
 * UART.h
 *
 *  Created on: Jun 3, 2025
 *      Author: Ecchs xanh UET
 */

#ifndef UART_H_
#define UART_H_

#include <stdint.h>

typedef struct
{
	volatile uint32_t CRL;
	volatile uint32_t CRH;
}GPIO_Typedef;

typedef struct
{
	volatile uint32_t SR;
	volatile uint32_t DR;
	volatile uint32_t BRR;
	volatile uint32_t CR1;
}UART_Typedef;

#define GPIOA		((GPIO_Typedef*)0x40010800)


#define UART1		((UART_Typedef*)0x40013800)
// CR1 Register bit
#define UE		(1 << 13)
#define TE		(1 << 3)
#define RE 		(1 << 2)
// SR Register bit
#define TXE		(1 << 7)

/*
 * @brief Cấp clock cho gpioA, uart1 và config input ouput cho RXTX
 * @param void
 * @retval none
 */
void UART1_gpio_init(void);
/*
 * @brief Set baud 9k6 và Enable RXTX,UART
 * @param void
 * @retval none
 */
void UART1_baud_init(void);
/*
 * @brief Gửi dữ liệu
 * @param ký tự kiểu char
 * @retval none
 */
void UART1_send_data(char data);

#endif /* UART_H_ */
