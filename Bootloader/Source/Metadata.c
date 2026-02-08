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

    Flash_WriteWord(addr, mtd->activeFirmwareStatus); addr += 4;
    Flash_WriteWord(addr, mtd->oldFirmwareStatus);    addr += 4;
    Flash_WriteWord(addr, mtd->trialBootFlag);        addr += 4;
    Flash_WriteWord(addr, mtd->trialBootCount);       addr += 4;
    Flash_WriteWord(addr, mtd->versionApp1);          addr += 4;
    Flash_WriteWord(addr, mtd->versionApp2);          addr += 4;
    Flash_WriteWord(addr, mtd->sizeApp1);             addr += 4;
    Flash_WriteWord(addr, mtd->sizeApp2);             addr += 4;
    Flash_WriteWord(addr, mtd->CRCApp1);              addr += 4;
    Flash_WriteWord(addr, mtd->CRCApp2);              addr += 4;
    Flash_WriteWord(addr, mtd->app1EntryAddr);        addr += 4;
    Flash_WriteWord(addr, mtd->app2EntryAddr);        addr += 4;
    Flash_WriteWord(addr, mtd->elapsedTime);
}

void Flash_readMetadata(Metadata_t* dst)
{
	uint32_t* mtd = (uint32_t*) METADATA_START_ADDR;
	dst->activeFirmwareStatus 	= mtd[0];
	dst->oldFirmwareStatus		= mtd[1];
	dst->trialBootFlag			= mtd[2];
	dst->trialBootCount			= mtd[3];
	dst->versionApp1			= mtd[4];
	dst->versionApp2			= mtd[5];
	dst->sizeApp1				= mtd[6];
	dst->sizeApp2				= mtd[7];
	dst->CRCApp1				= mtd[8];
	dst->CRCApp2				= mtd[9];
	dst->app1EntryAddr			= mtd[10];
	dst->app2EntryAddr			= mtd[11];
	dst->elapsedTime			= mtd[12];
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
