/*
 * Bootflags.h
 *
 *  Created on: Jul 6, 2025
 *      Author: ACER
 */

#ifndef BOOTFLAGS_H_
#define BOOTFLAGS_H_

#include <stdint.h>

typedef enum
{
	APP1_ACTIVE,
	APP1_OLD,
	APP2_ACTIVE,
	APP2_OLD,
	FIRSTRUN_FLAG,
	NONE_OLD_VER
} Flags_t;

// Flag address definition
#define BOOTLOADER_START_ADDR		0x08000000		// 8 page from 0x08000000 - 0x08001FFF
#define APP1_START_ADDR				0x08002000		// 20 page from 0x08002000 - 0x08006FFF
#define APP2_START_ADDR				0x08007000		// 20 page from 0x08007000 - 0x0800BFFF
#define METADATA_FLAGS_ADDR			0x0800C800		// 1kb (1page) for flags from 0x0800C800 - 0x0800CFFF
#define METADATA_PENDING_ADDR		(METADATA_FLAGS_ADDR + 0x400)
#define METADATA_COUNTING_ADDR		(METADATA_FLAGS_ADDR + 0x800)

#define UPDATEPENDING				1				// Cờ này bật lên khi vừa upfirmware lên, chờ chạy trial
#define UPDATECOMP					0				// Cờ này bật lên khi trial xong


#endif /* BOOTFLAGS_H_ */
