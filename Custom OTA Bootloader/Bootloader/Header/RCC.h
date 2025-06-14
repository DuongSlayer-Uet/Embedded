/*
 * RCC.h
 *
 *  Created on: May 27, 2025
 *      Author: ACER
 */

#ifndef RCC_H_
#define RCC_H_

#include <stdint.h>

typedef struct
{
	volatile uint32_t CR;			// Control Reg
	volatile uint32_t CFGR;			// Config Reg
	volatile uint32_t CIR;
	volatile uint32_t APB2RSTR;
	volatile uint32_t APB1RSTR;
	volatile uint32_t AHBENR;		// Enable Reg
	volatile uint32_t APB2ENR;
	volatile uint32_t APB1ENR;
}RCC_TypeDef;

typedef enum
{
	CLOCK_HSI_8MHZ,
	CLOCK_HSI_64MHZ,
	CLOCK_HSE_48MHZ,
	CLOCK_HSE_72MHZ
}ClockConfig_t;

// Thanh ghi cấu hình buffer cho flash khi chạy high speed
#define FLASH_ACR 		(*(volatile uint32_t*)0x40022000)
// ACR bit
#define PRFTBE 			(1 << 4)
#define LATENCY_ZERO	0				// 0-24mhz
#define LATENCY_ONE		(1 << 0)		// 24-48mhz
#define LATENCY_TWO		(1 << 1)		// 48-72mhz

// Các bit trong Struct của clock
#define RCC 			((RCC_TypeDef*)0x40021000)
// CR bit
#define HSION 			(1 << 0)
#define HSEON			(1 << 16)
#define PLLON			(1 << 24)
#define HSIRDY			(1 << 1)
#define HSERDY			(1 << 17)
#define PLLRDY			(1 << 25)
// CFGR bit
#define SW_HSI			0
#define SW_HSE			(1 << 0)
#define SW_PLL			(1 << 1)
#define PLLSRC			(1 << 16)	// This for HSE, HSE = PLL
#define PLLSRC_HSI		~(1 << 16)	// This for HSI
#define PLLX6			(1 << 20)
#define PLLX9			(0b0111 << 18)
#define PLLX16			(0b1111 << 18)

#define RCC_APB2ENR (*(volatile uint32_t*)0x40021018)		// RCC APB2 Enable register
/*
 * @brief Khởi tạo clock 48Mhz từ HSE
 * @param void
 * @retval none
 */
void RCC_Config_HSE_PLL_48MHz(void);

/*
 * @brief Khởi tạo clock 72Mhz từ HSE
 * @param void
 * @retval none
 */
void RCC_Config_HSE_PLL_72MHz(void);

/*
 * @brief Khởi tạo clock 8Mhz từ HSI
 * @param void
 * @retval none
 */
void RCC_Config_HSI_8MHz(void);

/*
 * @brief Khởi tạo clock 64Mhz từ HSI
 * @param void
 * @retval none
 */
void RCC_Config_HSI_PLL_64MHz(void);

/*
 * @brief Hàm khởi tạo clock từ tham số enum ClockConfig_t
 * @param clock: Lựa chọn theo định nghĩa trong enum ClockConfig_t
 * @retval none
 */
void RCC_Config(ClockConfig_t clock);
#endif /* RCC_H_ */
