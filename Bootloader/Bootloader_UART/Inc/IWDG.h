/*
 * IWDG.h
 *
 *  Created on: Jun 20, 2025
 *      Author: ACER
 */

#ifndef IWDG_H_
#define IWDG_H_

#include <stdint.h>

typedef struct
{
	volatile uint32_t KR;
	volatile uint32_t PR;
	volatile uint32_t RLR;
	volatile uint32_t SR;
}IWDG_t;

#define IWDG		((IWDG_t*)0x40003000)
/*
 * @brief Khởi tạo watchdog, set LSI 40khz, start IWDG
 * @param void
 * @retval none
 */
void IWDG_setup(void);
/*
 * @brief Hàm này dùng để write vào KR, refresh lại counter của watchdog
 * @param void
 * @retval none
 */
void IWDG_refresh(void);

#endif /* IWDG_H_ */
