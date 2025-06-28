/*
 * Bootflags.h
 *
 *  Created on: Jun 18, 2025
 *      Author: ACER
 */

#ifndef BOOTFLAGS_H_
#define BOOTFLAGS_H_

#include <stdint.h>

// This enum defines flags
enum bootflags
{
	FLAG_RUN_APP = 0,
	FLAG_RUN_FACTORY,
	FLAG_UPDATE_APP,
	FLAG_UPDATE_FACTORY,
	FLAG_START_WRITE_FACTORY,
	FLAG_START_WRITE_APP
};

// This struct support action decision
typedef struct
{
	uint32_t magic;
	uint16_t next_action;
} flags_t;

// Flag address definition
#define ADDR_BOOTLOADER_START		0x08000000		// 8 page from 0x08000000 - 0x08001FFF
#define ADDR_FACTORY_START			0x08002000		// 20 page from 0x08002000 - 0x08006FFF
#define ADDR_APP_START				0x08007000		// 20 page from 0x08007000 - 0x0800BFFF
#define ADDR_FLAGS_START			0x0800C800		// 1kb (1page) for flags from 0x0800C800 - 0x0800CFFF

#endif /* BOOTFLAGS_H_ */
