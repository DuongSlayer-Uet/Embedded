/*
 * Timer.h
 *
 *  Created on: May 26, 2025
 *      Author: EchXanhUET
 */

#ifndef TIMER_H_
#define TIMER_H_

#include <stdint.h>

typedef struct
{
	volatile uint32_t CR1;		// CONTROL REG1
	volatile uint32_t CR2;		// CONTROL REG2
	volatile uint32_t SMCR;
	volatile uint32_t DIER;
	volatile uint32_t SR;
	volatile uint32_t EGR;
	volatile uint32_t CCMR1;
	volatile uint32_t CCMR2;
	volatile uint32_t CCER;
	volatile uint32_t CNT;
	volatile uint32_t PSC;		// PRESCALER REG
	volatile uint32_t ARR;		// AUTO RE-LOAD REGISTER
}TIMX_TypeDef;

#define RCC_BASE_ADDRESS 0x40021000U

#define TIM1 				((TIMX_TypeDef*)0x40012C00U)



void setup_timer1(void);

void delay_1s(void);


#endif /* TIMER_H_ */
