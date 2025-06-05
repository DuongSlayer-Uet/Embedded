/*
 * PWM.h
 *
 *  Created on: Jun 5, 2025
 *      Author: ACER
 */

#ifndef PWM_H_
#define PWM_H_

#include <stdint.h>

/*
 * @brief: Bật clock cho TIM1 và GPIOA
 * @param none
 */
void Clock_Enable_PWM1();

/*
 * @brief: Set alternative output
 * @param none
 */
void GPIOA_Alternative_PushPull();

void Tim2_CH1_Setup();

#endif /* PWM_H_ */
