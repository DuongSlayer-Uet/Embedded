/*
 * IWDG.c
 *
 *  Created on: Jun 20, 2025
 *      Author: ACER
 */
#include "IWDG.h"
#include "RCC.h"

void IWDG_setup(void)
{
	RCC_Config_LSI_40K();
	// enter Key to access Keyregister
	IWDG->KR = 0x5555;

	// Pres = 256
	IWDG->PR |= (0b111 << 0);

	// Reload registerr = 1250 ~ 8s
	IWDG->RLR = 100;

	// Start IWDG
	IWDG->KR = 0xAAAA;
	IWDG->KR = 0xCCCC;	// bắt đầu
}

void IWDG_refresh(void)
{
	IWDG->KR = 0xAAAA;
}
