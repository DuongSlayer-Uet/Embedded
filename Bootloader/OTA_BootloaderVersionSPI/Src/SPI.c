/*
 * SPI.c
 *
 *  Created on: Aug 4, 2025
 *      Author: ACER
 */

#include "SPI.h"
#include "gpio.h"
#include "RCC.h"

void SPI1_MasterSetup(void)
{
	// CLOCK CONFIG
	// APB2 = 8mhz
	RCC_Config_HSI_8MHz();
	// Enable SPI1 clock (apb2), AFIO, GPIOA
	RCC->APB2ENR |= (1 << 12) | (1 << 0) | (1 << 2);
	// GPIO CONFIG
	// GPIOA7 - MOSI - Output pushpull max 10mhz
	GPIOA->CRL &= ~(0xF << 28);
	GPIOA->CRL |= (0b1001 << 28);
	// GPIOA6 - MISO - Input floating
	GPIOA->CRL &= ~(0xF << 24);
	GPIOA->CRL |= (0b0100 << 24);
	// GPIOA5 - CLK - Output pushpull max 10mhz
	GPIOA->CRL &= ~(0xF << 20);
	GPIOA->CRL |= (0b1001 << 20);
	// NSS - PA4 - AF push-pull max 10MHz
	GPIOA->CRL &= ~(0xF << 16);
	GPIOA->CRL |= (0b1001 << 16); // MODE=01 (10MHz), CNF=10 (AF Push-Pull)
	// SPI CONFIG
	// Reset CR1
	SPI1->CR1 = 0;
	// Master mode
	SPI1->CR1 |= MSTR;
	// CS hardware: disable software management
	SPI1->CR1 &= ~SSM;  // Tắt software NSS
	SPI1->CR1 &= ~SSI;  // Không giữ NSS mức cao giả lập
	// Cho phép xuất NSS từ hardware
	SPI1->CR2 |= (1 << 2); // SSOE = 1
	// Baud: 1.000.000 (APB2 = 8/8 = 1Mhz)
	SPI1->CR1 |= (0b010 << 3);
	// Clock pol
	SPI1->CR1 &= ~CPOL;
	// clock phase
	SPI1->CR1 &= ~CPHA;
	// 8 bit data
	SPI1->CR1 &= ~DFF;
	// MSB First
	SPI1->CR1 &= ~LSBFIRST;
	// ENable SPI1
	SPI1->CR1 |= SPE;
}

void SPI1_SlaveSetup(void)
{
    // CLOCK CONFIG
    RCC_Config_HSI_8MHz(); // APB2 = 8MHz
    // Enable SPI1 clock (APB2), AFIO, GPIOA
    RCC->APB2ENR |= (1 << 12) | (1 << 0) | (1 << 2);

    // GPIO CONFIG cho SPI slave
    // PA7 - MOSI (Master Out Slave In) - Input floating
    GPIOA->CRL &= ~(0xF << 28);
    GPIOA->CRL |= (0b0100 << 28);
    // PA6 - MISO (Master In Slave Out) - Output AF push-pull max 10MHz
    GPIOA->CRL &= ~(0xF << 24);
    GPIOA->CRL |= (0b1001 << 24);
    // PA5 - SCK - Input floating
    GPIOA->CRL &= ~(0xF << 20);
    GPIOA->CRL |= (0b0100 << 20);
    // PA4 - NSS - Input floating (hardware NSS)
    GPIOA->CRL &= ~(0xF << 16);
    GPIOA->CRL |= (0b0100 << 16);

    // SPI CONFIG
    SPI1->CR1 = 0;         // Reset CR1
    SPI1->CR1 &= ~MSTR;    // Slave mode
    SPI1->CR1 &= ~SSM;     // Dùng hardware NSS
    SPI1->CR1 &= ~SSI;     // Không giả lập NSS

    // CPOL = 0, CPHA = 1
    SPI1->CR1 &= ~CPOL;
    SPI1->CR1 |= CPHA;

    // 8-bit data
    SPI1->CR1 &= ~DFF;
    // MSB first
    SPI1->CR1 &= ~LSBFIRST;

    // RX enable (this bit will set when DR received fully 1 byte from shift register)
    // SPI1->CR2 |= (1 << 6); // RXNEIE = 1

    // Enable SPI1
    SPI1->CR1 |= SPE;
}

void SPI1_MasterTransfer(uint8_t data)
{
	// chờ buffer trống
	while((SPI1->SR & (1 << 1)) == 0);
	// write dr
	SPI1->DR = data;
	// Wait for rxne = 1
	while((SPI1->SR & (1 << 0)) == 0);
	// Read to reset rxne to 0
	volatile uint8_t dt = SPI1->DR;
	(void)dt;
}

uint8_t SPI1_SlaveTransfer(uint8_t data)
{
    // Ghi dữ liệu sẵn vào DR để master đọc khi clock tới
    SPI1->DR = data;
    // Chờ master gửi xung và byte mới vào
    while((SPI1->SR & (1 << 0)) == 0); // RXNE = 1 khi nhận xong
    return SPI1->DR;
}
