/*
 * CRC.c
 *
 *  Created on: Jul 3, 2025
 *      Author: ACER
 */

#include "CRC.h"
#include "RCC.h"

#include <stdint.h>

void CRC_Setup(void)
{
	// enable clock for RCC
	RCC->AHBENR |= (1 << 6);
	// Clear CRC data
	CRC->CR |= (1 << 0);
}

void CRC_WriteWord(uint32_t data)
{
	CRC->DR = data;
}

uint32_t CRC_GetResult(void)
{
	return (CRC->DR);
}
