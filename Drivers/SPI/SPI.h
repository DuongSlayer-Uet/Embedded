/*
 * SPI.h
 *
 *  Created on: Aug 4, 2025
 *      Author: ACER
 */

#ifndef SPI_H_
#define SPI_H_

#include <stdint.h>

typedef struct
{
	volatile uint32_t CR1;
	volatile uint32_t CR2;
	volatile uint32_t SR;
	volatile uint32_t DR;
}SPI_Typedef;

// CR1 reg
#define MSTR		(1 << 2)
#define SSM			(1 << 9)
#define SSI			(1 << 8)
#define CPOL		(1 << 1)
#define CPHA		(1 << 0)
#define DFF			(1 << 11)
#define LSBFIRST	(1 << 7)
#define SPE			(1 << 6)
#define RXDMAEN		(1 << 0)
#define TXDMAEN		(1 << 1)

#define SPI1			((SPI_Typedef*) 0x40013000ul)

/*
 * Brief: 	Setup SPI
 * 			Clock: HSI-APB2 - 8Mhz
 * 			GPIO5-7
 * 			Baud 1000000
 * 			8 bit data
 * Retval: void
 */
void SPI1_MasterSetup(void);

void SPI1_MasterTransfer(uint8_t data);

void SPI1_SlaveSetup(void);

uint8_t SPI1_SlaveTransfer(uint8_t data);
#endif /* SPI_H_ */
