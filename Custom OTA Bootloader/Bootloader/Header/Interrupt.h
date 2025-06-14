/*
 * Interrupt.h
 *
 *  Created on: May 24, 2025
 *      Author: ÉchxanhUet :>
 */

#ifndef INTERRUPT_H_
#define INTERRUPT_H_

#include <stdint.h>
#include "gpio.h"


typedef struct
{
	volatile uint32_t IMR;
	volatile uint32_t EMR;
	volatile uint32_t RTSR;
	volatile uint32_t FTSR;
	volatile uint32_t SWIER;
	volatile uint32_t PR;
} EXTI_TypeDef;

typedef struct
{
	volatile uint32_t EVCR;
	volatile uint32_t MAPR;
	volatile uint32_t EXTICR1;
	volatile uint32_t EXTICR2;
	volatile uint32_t EXTICR3;
	volatile uint32_t EXTICR4;
	volatile uint32_t MAPR2;
} AFIO_TypeDef;

// FROM NVIC TABLE
#define EXTI0_IRQ_NUM 		6
#define EXTI1_IRQ_NUM 		7
#define EXTI2_IRQ_NUM 		8
#define EXTI3_IRQ_NUM 		9
#define EXTI4_IRQ_NUM 		10
#define EXTI5_9_IRQ_NUM 	23
#define EXTI10_15_IRQ_NUM	40
#define USART1				37

// core_level
#define NVIC_ISER_BASEADRESS 	0xE000E100U
#define NVIC_ISER0 				(*(volatile uint32_t*)(NVIC_ISER_BASEADRESS + 0X00))		// For EXTI0->9
#define NVIC_ISER1 				(*(volatile uint32_t*)(NVIC_ISER_BASEADRESS + 0X04))		// For EXTI10->15

// peripherals_level
#define EXTI ((EXTI_TypeDef*)0x40010400U)
#define AFIO ((AFIO_TypeDef*)0x40010000U)

/*
 * @brief Enable ngắt ở trong core arm, cho phép ngắt ở NVIC
 * @param EXTI_IRQ_NUM: Mã số ngắt trong bảng NVIC (pos number)
 * @retval none
 */
void NVIC_Enable_IRQ(uint8_t EXTI_IRQ_NUM);

/*
 * @brief Enable ngắt ở bên ngoài core, bao gồm unmask, enable exti line,...
 * @param gpio: GPIOA, GPIOB,...
 * @param pin: 0-15
 * @param edgetype: 0-falling 1-rising
 * @retval none
 */
void EXTI_Init(GPIO_Typedef* gpio, uint8_t pin, uint8_t edge_type);

void USART1_RX_Int_init(void);

/*
 * @brief Xử lý ngắt, xóa cờ ngắt bằng cách write 1 vào Pending register
 * @param void
 * @retval none
 */
void EXTI0_IRQHandler(void);
void EXTI1_IRQHandler(void);
void EXTI2_IRQHandler(void);
void EXTI3_IRQHandler(void);
void EXTI4_IRQHandler(void);
void EXTI9_5_IRQHandler(void);
void EXTI15_10_IRQHandler(void);
void USART1_IRQHandler(void);
#endif /* INTERRUPT_H_ */
