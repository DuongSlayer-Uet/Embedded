/*
 * PWM.c
 *
 *  Created on: Jun 5, 2025
 *      Author: ACER
 */

#include "PWM.h"
#include "RCC.h"
#include "gpio.h"
#include "Timer.h"

void Clock_Enable_PWM1()
{
	// Enable GPIOA and TIM2 Clock
	RCC->APB2ENR |= (1 << 2);

	RCC->APB1ENR |= (1 << 0);
}

void GPIOA_Alternative_PushPull()
{
	// PA0, alternative func, Output, max 10mhz, push pull
	GPIOA->CRL &= ~(0xF << 0);
	GPIOA->CRL |= (0b1011 << 0);
}

void Tim2_CH1_Setup()
{
	// Prescaler = 8
	TIM2->PSC = 7;
	// Autoreload = 20000
	TIM2->ARR = 19999;
    // Cấu hình chế độ PWM Mode 1 cho kênh 1
    TIM2->CCMR1 &= ~(0b11 << 0);        // Chọn chế độ Output
    TIM2->CCMR1 |= (0b110 << 4);        // PWM mode 1
    TIM2->CCMR1 |= (1 << 3);            // Cho phép preload cho CCR1
    // Enable channel 1
    TIM2->CCER |= (1 << 0);
    TIM2->CR1 |= (1 << 7);          // ARPE: preload ARR
    // Dutycycle = 10000 (50%)
    TIM2->CCR1 = 10000;
    // Counter Enable
    TIM2->CR1 |= (1 << 0);
}
