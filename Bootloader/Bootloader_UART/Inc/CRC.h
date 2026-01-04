/*
 * CRC.h
 *
 *  Created on: Jul 3, 2025
 *      Author: ACER
 */

#ifndef CRC_H_
#define CRC_H_

#include <stdint.h>

typedef struct
{
	volatile uint32_t DR;
	volatile uint32_t IDR;
	volatile uint32_t CR;
} CRC_typedef_t;

#define CRC		((CRC_typedef_t*) 0x40023000)

void CRC_Setup(void);

void CRC_WriteWord(uint32_t data);

uint32_t CRC_ReadEncriptedData(void);

uint32_t CRC_GetResult(void);
#endif /* CRC_H_ */
