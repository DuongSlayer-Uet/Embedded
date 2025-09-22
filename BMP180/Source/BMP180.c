/*
 * I2C.c
 *
 *  Created on: Aug 19, 2025
 *      Author: ACER
 */

#include <BMP180.h>
#include "gpio.h"
#include "RCC.h"
#include <stdint.h>

void I2C1_SlaveConfig(void)
{
	// RCC enable
	RCC->APB2ENR |= (1 << 3);		// IOB

	// IO config (output, max 2mhz, opendrain, AF mode
	GPIOB->CRL &= ~(0b1111 << 24);
	GPIOB->CRL |= (0b1110 << 24);	// IOB6 config
	GPIOB->CRL &= ~(0b1111 << 28);
	GPIOB->CRL |= (0b1110 << 28);	// IOB7 config

	// I2C config
	RCC->APB1ENR |= (1 << 21);		// I2C

	I2C1->CR1 |= (1 << 15);			// Reset
	I2C1->CR1 &= ~(1 << 15);		//

	I2C1->CR2 = 8;					// Freq = 8mhz

	I2C1->OAR1 = (0x32 << 1) | (1 << 14);		// Slave addr = 0x32

	I2C1->CR1 |= (1 << 10);			// ACK enable
	I2C1->CR1 |= (1 << 0);			// Peripheral enable
}

void I2C1_MasterConfig(void)
{
	// 1. Bật clock

	RCC->APB2ENR |= (1 << 3);		// GPIOB

	// 2. Cấu hình PB6 (SCL), PB7 (SDA) = Alternate function, Open-drain, 2MHz
	GPIOB->CRL &= ~(0xF << 24);		// PB6
	GPIOB->CRL |=  (0xF << 24);		// AF OD, output 2MHz
	GPIOB->CRL &= ~(0xF << 28);		// PB7
	GPIOB->CRL |=  (0xF << 28);

	// 3. Reset I2C1
	RCC->APB1ENR |= (1 << 21);		// I2C1

	I2C1->CR1 |=  (1 << 15);
	I2C1->CR1 &= ~(1 << 15);

	// 4. Cấu hình CR2 (input clock frequency in MHz)
	I2C1->CR2 = 8;	// PCLK1 = 8MHz

	// 5. Set tốc độ SCL = 100kHz
	// Standard mode: CCR = Fpclk1 / (2*Fscl) = 8MHz / (2*100kHz) = 40
	I2C1->CCR = 40;
	I2C1->TRISE = 9;   // (1000ns / Tpclk1) + 1 = (1000ns / 125ns) + 1 = 8 + 1

	// 6. Enable I2C peripheral
	I2C1->CR1 |= (1 << 0);
}

uint8_t I2C1_Write8(uint8_t devAddr, uint8_t regAddr, uint8_t regVal)
{
	// Start
	I2C1->CR1 |= I2C_CR1_START;
	// wait until start bit is sent
	while(!(I2C1->SR1 & I2C_SR1_SB));
	// devaddr + write bit (0)
	I2C1->DR = (devAddr << 1);
	// wait until addr bit is sent
	while(!(I2C1->SR1 & (I2C_SR1_ADDR | I2C_SR1_AF)));
	// check if ack is received
	if(I2C1->SR1 & I2C_SR1_AF)
	{
		I2C1->SR1 &= ~I2C_SR1_AF;
		I2C1->CR1 |= I2C_CR1_STOP;
		return 0;
	}
	// read SR1 sr2 to clear addr flag
    (void)I2C1->SR1;
    (void)I2C1->SR2;

    // Send register address
    I2C1->DR = regAddr;
    while(!(I2C1->SR1 & (I2C_SR1_TXE | I2C_SR1_AF)));
    if(I2C1->SR1 & I2C_SR1_AF)
    {
        I2C1->SR1 &= ~I2C_SR1_AF;
        I2C1->CR1 |= I2C_CR1_STOP;
        return 0;
    }
    // Send register value
    I2C1->DR = regVal;
    while(!(I2C1->SR1 & (I2C_SR1_TXE | I2C_SR1_AF)));
    if(I2C1->SR1 & I2C_SR1_AF)
    {
        I2C1->SR1 &= ~I2C_SR1_AF;
        I2C1->CR1 |= I2C_CR1_STOP;
        return 0;
    }
    // STOP
    I2C1->CR1 |= I2C_CR1_STOP;
    return 1;
}

uint8_t I2C1_Read8(uint8_t devAddr, uint8_t regAddr)
{
	// Start
	I2C1->CR1 |= I2C_CR1_START;
	// wait until start bit is sent
	while(!(I2C1->SR1 & I2C_SR1_SB));
	// devaddr + write (0)
	I2C1->DR = (devAddr << 1);
	// wait until addr bit is sent
	while(!(I2C1->SR1 & (I2C_SR1_ADDR | I2C_SR1_AF)));
	// check if ack is received
	if(I2C1->SR1 & I2C_SR1_AF)
	{
		I2C1->SR1 &= ~I2C_SR1_AF;
		I2C1->CR1 |= I2C_CR1_STOP;
		return 0;
	}
	// read SR1 sr2 to clear addr flag
    (void)I2C1->SR1;
    (void)I2C1->SR2;

    // Send register address
    I2C1->DR = regAddr;
    while(!(I2C1->SR1 & (I2C_SR1_TXE | I2C_SR1_AF)));
    if(I2C1->SR1 & I2C_SR1_AF)
    {
        I2C1->SR1 &= ~I2C_SR1_AF;
        I2C1->CR1 |= I2C_CR1_STOP;
        return 0;
    }

	// REStart
	I2C1->CR1 |= I2C_CR1_START;
	// wait until start bit is sent
	while(!(I2C1->SR1 & I2C_SR1_SB));
	// devaddr + read(1)
	I2C1->DR = (devAddr << 1) | 1;
	// wait until addr bit is sent
	while(!(I2C1->SR1 & (I2C_SR1_ADDR | I2C_SR1_AF)));
	// check if ack is received
	if(I2C1->SR1 & I2C_SR1_AF)
	{
		I2C1->SR1 &= ~I2C_SR1_AF;
		I2C1->CR1 |= I2C_CR1_STOP;
		return 0;
	}
	// read SR1 sr2 to clear addr flag
    (void)I2C1->SR1;
    (void)I2C1->SR2;

    // NACK last byte
    I2C1->CR1 &= ~I2C_CR1_ACK;
    I2C1->CR1 |= I2C_CR1_STOP;

    while(!(I2C1->SR1 & I2C_SR1_RXNE));
    uint8_t ID = I2C1->DR;
    return ID;
}

uint16_t I2C1_Read16(uint8_t devAddr, uint8_t regAddr)
{
	// Start
	I2C1->CR1 |= I2C_CR1_START;
	// wait until start bit is sent
	while(!(I2C1->SR1 & I2C_SR1_SB));
	// devaddr + write (0)
	I2C1->DR = (devAddr << 1);
	// wait until addr bit is sent
	while(!(I2C1->SR1 & (I2C_SR1_ADDR | I2C_SR1_AF)));
	// check if ack is received
	if(I2C1->SR1 & I2C_SR1_AF)
	{
		I2C1->SR1 &= ~I2C_SR1_AF;
		I2C1->CR1 |= I2C_CR1_STOP;
		return 0;
	}
	// read SR1 sr2 to clear addr flag
    (void)I2C1->SR1;
    (void)I2C1->SR2;

    // Send register address
    I2C1->DR = regAddr;
    while(!(I2C1->SR1 & (I2C_SR1_TXE | I2C_SR1_AF)));
    if(I2C1->SR1 & I2C_SR1_AF)
    {
        I2C1->SR1 &= ~I2C_SR1_AF;
        I2C1->CR1 |= I2C_CR1_STOP;
        return 0;
    }

	// REStart
	I2C1->CR1 |= I2C_CR1_START;
	// wait until start bit is sent
	while(!(I2C1->SR1 & I2C_SR1_SB));
	// devaddr + read(1)
	I2C1->DR = (devAddr << 1) | 1;
	// wait until addr bit is sent
	while(!(I2C1->SR1 & (I2C_SR1_ADDR | I2C_SR1_AF)));
	// check if ack is received
	if(I2C1->SR1 & I2C_SR1_AF)
	{
		I2C1->SR1 &= ~I2C_SR1_AF;
		I2C1->CR1 |= I2C_CR1_STOP;
		return 0;
	}

    // ACK + stop
    I2C1->CR1 &= ~I2C_CR1_ACK;

	// read SR1 sr2 to clear addr flag
    (void)I2C1->SR1;
    (void)I2C1->SR2;

    // MSB
    while(!(I2C1->SR1 & I2C_SR1_RXNE));
    uint8_t msb = I2C1->DR;

    // LSB
    while(!(I2C1->SR1 & I2C_SR1_RXNE));
    uint8_t lsb = I2C1->DR;

    uint16_t value = ((uint16_t)msb << 8) | lsb;
    return value;
}
