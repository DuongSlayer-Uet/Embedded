/*
 * BMP180_I2C1.c
 *
 *  Created on: Aug 19, 2025
 *      Author: ACER
 */

#include <BMP180.h>
#include "gpio.h"
#include "RCC.h"
#include <stdint.h>

void BMP180_I2C1_SlaveConfig(void)
{
	// RCC enable
	RCC->APB2ENR |= (1 << 3);		// IOB

	// IO config (output, max 2mhz, opendrain, AF mode
	GPIOB->CRL &= ~(0b1111 << 24);
	GPIOB->CRL |= (0b1110 << 24);	// IOB6 config
	GPIOB->CRL &= ~(0b1111 << 28);
	GPIOB->CRL |= (0b1110 << 28);	// IOB7 config

	// BMP180_I2C1 config
	RCC->APB1ENR |= (1 << 21);		// BMP180_I2C1

	BMP180_I2C1->CR1 |= (1 << 15);			// Reset
	BMP180_I2C1->CR1 &= ~(1 << 15);		//

	BMP180_I2C1->CR2 = 8;					// Freq = 8mhz

	BMP180_I2C1->OAR1 = (0x32 << 1) | (1 << 14);		// Slave addr = 0x32

	BMP180_I2C1->CR1 |= (1 << 10);			// ACK enable
	BMP180_I2C1->CR1 |= (1 << 0);			// Peripheral enable
}

void BMP180_I2C1_MasterConfig(void)
{
	// 1. Bật clock

	RCC->APB2ENR |= (1 << 3);		// GPIOB

	// 2. Cấu hình PB6 (SCL), PB7 (SDA) = Alternate function, Open-drain, 2MHz
	GPIOB->CRL &= ~(0xF << 24);		// PB6
	GPIOB->CRL |=  (0xF << 24);		// AF OD, output 2MHz
	GPIOB->CRL &= ~(0xF << 28);		// PB7
	GPIOB->CRL |=  (0xF << 28);

	// 3. Reset BMP180_I2C1
	RCC->APB1ENR |= (1 << 21);		// BMP180_I2C1

	BMP180_I2C1->CR1 |=  (1 << 15);
	BMP180_I2C1->CR1 &= ~(1 << 15);

	// 4. Cấu hình CR2 (input clock frequency in MHz)
	BMP180_I2C1->CR2 = 8;	// PCLK1 = 8MHz

	// 5. Set tốc độ SCL = 100kHz
	// Standard mode: CCR = Fpclk1 / (2*Fscl) = 8MHz / (2*100kHz) = 40
	BMP180_I2C1->CCR = 40;
	BMP180_I2C1->TRISE = 9;   // (1000ns / Tpclk1) + 1 = (1000ns / 125ns) + 1 = 8 + 1

	// 6. Enable BMP180_I2C1 peripheral
	BMP180_I2C1->CR1 |= (1 << 0);
}

uint8_t BMP180_I2C1_Write8(uint8_t devAddr, uint8_t regAddr, uint8_t regVal)
{
    // START
    BMP180_I2C1->CR1 |= BMP180_I2C1_CR1_START;                 // Generate START
    while (!(BMP180_I2C1->SR1 & BMP180_I2C1_SR1_SB));          // EV5: SB=1
    (void)BMP180_I2C1->SR1;                            // Clear SB by read SR1
    BMP180_I2C1->DR = (devAddr << 1);                  // Send slave addr + W (LSB=0)

    //EV6: ADDR sent
    while (!(BMP180_I2C1->SR1 & BMP180_I2C1_SR1_ADDR));        // Wait for ADDR=1
    (void)BMP180_I2C1->SR1; (void)BMP180_I2C1->SR2;           // Clear ADDR by read SR1 + SR2

    //EV8_1: TxE=1, DR empty
    while (!(BMP180_I2C1->SR1 & BMP180_I2C1_SR1_TXE));         // Wait TXE=1
    BMP180_I2C1->DR = regAddr;                         // Send register address

    //EV8: TxE=1
    while (!(BMP180_I2C1->SR1 & BMP180_I2C1_SR1_TXE));
    BMP180_I2C1->DR = regVal;                          // Send data

    //EV8_2: TxE=1, BTF=1
    while (!(BMP180_I2C1->SR1 & BMP180_I2C1_SR1_BTF));         // Wait until byte transfer finished

    //STOP
    BMP180_I2C1->CR1 |= BMP180_I2C1_CR1_STOP;                  // Generate STOP

    return 0;   // OK
}

uint8_t BMP180_I2C1_Read8(uint8_t devAddr, uint8_t regAddr)
{
    uint8_t data;

    //START (EV5)
    BMP180_I2C1->CR1 |= BMP180_I2C1_CR1_START;
    while (!(BMP180_I2C1->SR1 & BMP180_I2C1_SR1_SB));   // Wait for SB=1
    (void)BMP180_I2C1->SR1;
    BMP180_I2C1->DR = (devAddr << 1);           // Gửi địa chỉ + Write (0)

    //EV6 (ADDR=1)
    while (!(BMP180_I2C1->SR1 & BMP180_I2C1_SR1_ADDR));
    (void)BMP180_I2C1->SR1; (void)BMP180_I2C1->SR2;

    //regAddr cần đọc
    while (!(BMP180_I2C1->SR1 & BMP180_I2C1_SR1_TXE));  // TXE=1
    BMP180_I2C1->DR = regAddr;
    while (!(BMP180_I2C1->SR1 & BMP180_I2C1_SR1_BTF));  // Đợi truyền xong

    //Re-START (EV5 lần 2)
    BMP180_I2C1->CR1 |= BMP180_I2C1_CR1_START;
    while (!(BMP180_I2C1->SR1 & BMP180_I2C1_SR1_SB));
    (void)BMP180_I2C1->SR1;
    BMP180_I2C1->DR = (devAddr << 1) | 0x01;    // Địa chỉ + Read (1)

    //EV6 + EV6_1
    while (!(BMP180_I2C1->SR1 & BMP180_I2C1_SR1_ADDR));
    BMP180_I2C1->CR1 &= ~BMP180_I2C1_CR1_ACK;           // NACK cho byte cuối
    (void)BMP180_I2C1->SR1; (void)BMP180_I2C1->SR2;

    BMP180_I2C1->CR1 |= BMP180_I2C1_CR1_STOP;           // STOP ngay sau ADDR clear

    //EV7 (RxNE=1)
    while (!(BMP180_I2C1->SR1 & BMP180_I2C1_SR1_RXNE));
    data = BMP180_I2C1->DR;                     // Đọc dữ liệu

    return data;
}

uint16_t BMP180_I2C1_Read16(uint8_t devAddr, uint8_t regAddr)
{
    // START1: WRITE REGADDR
    BMP180_I2C1->CR1 |= BMP180_I2C1_CR1_START;                      // Start
    while(!(BMP180_I2C1->SR1 & BMP180_I2C1_SR1_SB));                // EV5
    (void)BMP180_I2C1->SR1;// EV5
    BMP180_I2C1->DR = (devAddr << 1);                       // Slave addr + Write (0)
    while(!(BMP180_I2C1->SR1 & BMP180_I2C1_SR1_ADDR));              // EV6
    (void)BMP180_I2C1->SR1;
    (void)BMP180_I2C1->SR2;

    while(!(BMP180_I2C1->SR1 & BMP180_I2C1_SR1_TXE))
    	;               // EV8_1
    BMP180_I2C1->DR = regAddr;                              // Gửi regAddr
    while(!(BMP180_I2C1->SR1 & BMP180_I2C1_SR1_TXE))
    	;               // EV8

	// Enable ACK + POS
	BMP180_I2C1->CR1 |= BMP180_I2C1_CR1_ACK | BMP180_I2C1_CR1_POS;


	// START2: READ 2 bytes
	BMP180_I2C1->CR1 |= BMP180_I2C1_CR1_START;
	while(!(BMP180_I2C1->SR1 & BMP180_I2C1_SR1_SB));   // EV5
	(void)BMP180_I2C1->SR1;// EV5
	// send address+R
	BMP180_I2C1->DR = (devAddr << 1) | 1;
	// Đợi addr
	while(!(BMP180_I2C1->SR1 & BMP180_I2C1_SR1_ADDR)); // EV6
	(void)BMP180_I2C1->SR1; 	// clear addr
	(void)BMP180_I2C1->SR2;   // clear ADDR
	// Ngay sau khi clear add -> clear ack
	BMP180_I2C1->CR1 &= ~BMP180_I2C1_CR1_ACK;			// EV6_1
	// Đợi BTF = 1 (2 bytes về ok)
	while(!(BMP180_I2C1->SR1 & BMP180_I2C1_SR1_BTF));  // EV7_3
	// Stop
	BMP180_I2C1->CR1 |= BMP180_I2C1_CR1_STOP;
	uint8_t data1 = BMP180_I2C1->DR;
	uint8_t data2 = BMP180_I2C1->DR;
	uint16_t data = (data1 << 8) | data2;
	return data;
}
