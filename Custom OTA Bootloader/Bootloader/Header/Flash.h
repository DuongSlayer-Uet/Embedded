/*
 * Flash.h
 *
 *  Created on: Jun 7, 2025
 *      Author: Ech xanh UET
 */

#ifndef FLASH_H_
#define FLASH_H_

#include <stdint.h>

typedef struct
{
	volatile uint32_t ACR;
	volatile uint32_t KEYR;
	volatile uint32_t OPTKEYR;
	volatile uint32_t SR;
	volatile uint32_t CR;
	volatile uint32_t AR;
	volatile uint32_t OBR;
	volatile uint32_t WRPR;
} FLASH_TypeDef;

#define FLASH 		((FLASH_TypeDef*)0x40022000ul)

// CR Register
#define CR_LOCK		(1 << 7)
#define CR_PG		(1 << 0)
#define CR_PER		(1 << 1)
#define CR_STRT		(1 << 6)

// SR Register
#define SR_BSY		(1 << 0)

void Flash_WriteHalfWord(uint32_t address, uint16_t data);

void Flash_EraseOnePage(uint32_t address);

uint16_t Flash_ReadHalfWord(uint32_t address);

#endif /* FLASH_H_ */
