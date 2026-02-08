/*
 * Flash.c
 *
 *  Created on: Jun 7, 2025
 *      Author: Ech Xanh UET
 */

#include "Flash.h"

void Flash_ConfigHighSpeed(void)
{
	FLASH->ACR |= CR_PREFETCH;
	FLASH->ACR &= ~CR_LATENCY;
	FLASH->ACR |= CR_LATENCY;
}

void Flash_WriteHalfWord(uint32_t address, uint16_t data)
{
	// Chờ hết busy
	while(FLASH->SR & SR_BSY);

	// Check xem mở khóa chưa
	// key ở manual
	if(FLASH->CR & CR_LOCK)
	{
		FLASH->KEYR = 0x45670123;	// key1
		FLASH->KEYR = 0xCDEF89AB;	// key2
	}

	FLASH->CR |= CR_PG;

	*(volatile uint16_t*)address = data;

	// Chờ hết busy
	while(FLASH->SR & SR_BSY);

	// Unable write
	FLASH->CR &= ~CR_PG;

	// LOCK
	FLASH->CR |= CR_LOCK;
}

void Flash_EraseOnePage(uint32_t address)
{
	// Chờ hết busy
	while(FLASH->SR & SR_BSY);

	// Check xem mở khóa chưa
	// key ở manual
	if(FLASH->CR & CR_LOCK)
	{
		FLASH->KEYR = 0x45670123;	// key1
		FLASH->KEYR = 0xCDEF89AB;	// key2
	}

	// Xóa 1 page
	FLASH->CR |= CR_PER;

	// Page Address
	FLASH->AR = address;

	// Start erase
	FLASH->CR |= CR_STRT;

	// Chờ hết busy
	while(FLASH->SR & SR_BSY);

	// Disable Xóa 1 page
	FLASH->CR &= ~CR_PER;

	// LOCK
	FLASH->CR |= CR_LOCK;
}

uint16_t Flash_ReadHalfWord(uint32_t address)
{
	return (*(volatile uint16_t*)address);
}

uint32_t Flash_ReadWord(uint32_t address)
{
	return (*(volatile uint32_t*)address);
}

void Flash_eraseMultiplePage(uint32_t address, uint16_t num)
{
	for (int i = 0; i < num; i++)		// Xóa firmware lỗi (app2)
	{
		Flash_EraseOnePage(address + i * 1024);
	}
}

uint8_t Flash_ReadByte(uint32_t address)
{
	return (*(volatile uint8_t*)address);
}

void Flash_WriteWord(uint32_t address, uint32_t data)
{
	uint16_t lower = (uint16_t)(data & 0xFFFF);
	uint16_t upper = (uint16_t)((data >> 16) & 0xFFFF);
	Flash_WriteHalfWord(address, lower);
	Flash_WriteHalfWord(address + 2, upper);
}
