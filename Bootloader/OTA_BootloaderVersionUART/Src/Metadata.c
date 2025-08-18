/*
 * Metadata.c
 *
 *  Created on: Jul 19, 2025
 *      Author: ACER
 */

#include "Metadata.h"
#include "Flash.h"

void Flash_eraseMetadata(void)
{
	Flash_EraseOnePage(METADATA_START_ADDR);
}

void Flash_writeMetadata(Metadata_t* mtd)
{
	uint32_t addr = METADATA_START_ADDR;
	Flash_WriteHalfWord(addr, mtd->activeFirmwareStatus);
	addr += 2;
	Flash_WriteHalfWord(addr, mtd->oldFirmwareStatus);
	addr += 2;
	Flash_WriteHalfWord(addr, mtd->trialBootFlag);
	addr += 2;
	Flash_WriteHalfWord(addr, mtd->trialBootCount);
	addr += 2;
	Flash_WriteHalfWord(addr, mtd->versionApp1);
	addr += 2;
	Flash_WriteHalfWord(addr, mtd->versionApp2);
}

void Flash_readMetadata(Metadata_t* dst)
{
	uint16_t* mtd = (uint16_t*) METADATA_START_ADDR;
	dst->activeFirmwareStatus 	= mtd[0];
	dst->oldFirmwareStatus		= mtd[1];
	dst->trialBootFlag			= mtd[2];
	dst->trialBootCount			= mtd[3];
	dst->versionApp1			= mtd[4];
	dst->versionApp2			= mtd[5];
}

uint16_t Flash_encodeVer16bit(uint8_t major, uint8_t minor, uint8_t patch)
{
	return (major << 10) | (minor << 5) | patch;
}

void Flash_decodeVer16bit(uint16_t version, uint8_t* major, uint8_t* minor, uint8_t* patch)
{
	*major = version >> 10;
	*minor = (version >> 5) & 0x1F;
	*patch = version & 0x1F;
}
