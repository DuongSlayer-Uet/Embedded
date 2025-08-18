/*
 * Metadata.h
 *
 *  Created on: Jul 19, 2025
 *      Author: ACER
 */

#ifndef METADATA_H_
#define METADATA_H_

#include <stdint.h>

/////////////////////////////
/// FLASH PARTITION/////////
///////////////////////////
#define BOOTLOADER_START_ADDR		0x08000000		// 8 page from 0x08000000 - 0x08001FFF
#define APP1_START_ADDR				0x08002000		// 20 page from 0x08002000 - 0x08006FFF
#define APP2_START_ADDR				0x08007000		// 20 page from 0x08007000 - 0x0800BFFF
#define METADATA_START_ADDR			0x0800C800		// 1 page from 0x0800C800 - 0x0800CFFF

// Boot thử tối da 5 lần
#define MAX_TRIAL_BOOT			5
// START trial mode
#define TRIAL_MODE_START		1
// STOP trial mode
#define TRIAL_MODE_STOP			0

typedef enum
{
	FIRST_RUN,
	APP1_ACTIVE,
	APP1_OLD,
	APP2_ACTIVE,
	APP2_OLD,
	NO_OLD_VER
} FirmwareStatus_t;

typedef struct
{
	uint16_t activeFirmwareStatus;
	uint16_t oldFirmwareStatus;
	uint16_t trialBootFlag;
	uint16_t trialBootCount;
	uint16_t versionApp1;
	uint16_t versionApp2;
	uint16_t elapsedTime;
} Metadata_t;

#define metadata					((volatile Metadata_t*)METADATA_START_ADDR)

void Flash_eraseMetadata(void);

void Flash_writeMetadata(Metadata_t* mtd);

void Flash_readMetadata(Metadata_t* dst);

uint16_t Flash_encodeVer16bit(uint8_t major, uint8_t minor, uint8_t patch);

void Flash_decodeVer16bit(uint16_t version, uint8_t* major, uint8_t* minor, uint8_t* patch);

#endif /* METADATA_H_ */
