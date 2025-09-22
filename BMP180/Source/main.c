#include <stdint.h>
#include "RCC.h"
#include "BMP180.h"
#include "Timer.h"


int main(void)
{
	RCC_Config_HSI_8MHz();
	RCC_APB2ENR |= (1 << 11);
	setup_timer1();
	I2C1->CR1 |= (1 << 15);
	I2C1->CR1 &= ~(1 << 15);
	I2C1_MasterConfig();
	I2C1_Read8(0x77, 0xD0);

	I2C1_Write8(0x77, 0xF4, 0x2E);
	delay_ms(100);
	uint8_t msb = I2C1_Read8(0x77, 0xF6);
	uint8_t lsb = I2C1_Read8(0x77, 0xF7);
	uint16_t value = ((uint16_t)msb << 8) | lsb;
}
