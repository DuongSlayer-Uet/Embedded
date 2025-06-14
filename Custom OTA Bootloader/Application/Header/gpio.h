/*
 * gpio.h
 *
 *  Created on: May 23, 2025
 *      Author: ACER
 */

#ifndef GPIO_H_
#define GPIO_H_

#include <stdint.h>

// GPIO Registers struct
typedef struct
{
	volatile uint32_t CRL;
	volatile uint32_t CRH;
	volatile uint32_t IDR;
	volatile uint32_t ODR;
	volatile uint32_t BSRR;
	volatile uint32_t BRR;
	volatile uint32_t LCKR;
} GPIO_Typedef;

// GPIO Address
#define GPIOA ((GPIO_Typedef*)0x40010800)
#define GPIOB ((GPIO_Typedef*)0x40010C00)
#define GPIOC ((GPIO_Typedef*)0x40011000)
#define GPIOD ((GPIO_Typedef*)0x40011400)



/*
 * @brief Khởi tạo GPIO (Clock for GPIO)
 * @param gpio: con trỏ trỏ đến port A, B, C, D (for example: GPIOA)
 * @param pin: pin thứ ... trên port đã chọn
 * @retval none
 */
void GPIO_init_output(GPIO_Typedef* gpio, uint8_t pin);
/*
 * @brief Set giá trị cho port đã gọi hàm GPIO_init_output
 * @param gpio: con trỏ trỏ đến port A, B, C, D (for example: GPIOA)
 * @param pin: pin thứ ... trên port đã chọn
 * @retval none
 */
void GPIO_set(GPIO_Typedef* gpio, uint8_t pin);
/*
 * @brief Reset giá trị cho port đã gọi hàm GPIO_init_output
 * @param gpio: con trỏ trỏ đến port A, B, C, D (for example: GPIOA)
 * @param pin: pin thứ ... trên port đã chọn
 * @retval none
 */
void GPIO_reset(GPIO_Typedef* gpio, uint8_t pin);
/*
 * @brief Đảo trạng thái pinx gpioy
 * @param gpio: con trỏ trỏ đến port A, B, C, D (for example: GPIOA)
 * @param pin: pin thứ ... trên port đã chọn
 * @retval none
 */
void GPIO_toggle_pin(GPIO_Typedef* gpio, uint8_t pin);

#endif /* GPIO_H_ */
