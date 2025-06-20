/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Ech xanh uet
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include <stdint.h>
#include <string.h>
#include "Flash.h"
#include "DMA.h"
#include "UART.h"
#include "RCC.h"
#include "Ringbuffer.h"
#include "gpio.h"
#include "Interrupt.h"
#include "Bootflags.h"


// Flag
volatile flags_t* flags = ((flags_t*)ADDR_FLAGS_START);
// Factory Reset Handler function pointer
typedef void (*factoryFuncPointer)(void);
// App Reset Handler Function Pointer
typedef void (*appFuncPointer)(void);

// Function prototype declaration
// Init
void Initialization(void);
// Update Factory firmware
void UpdateFactoryHandler(void);
// Update App firmware
void UpdateApplicationHandler(void);
// Jump to factory firmware
void JumpToFactoryFirmware(void);
// Jump to application firmware
void JumpToAppFirmware(void);
// Hàm update firmware
void UpdateFirmware(uint32_t address, uint16_t next_action_flag);
// Đẩy mảng sang trái
void push_char_to_window(char new_char);

// Cái này để fix lỗi cho cái STOP Bug, đảm bảo nhận đúng kí tự
#define CMD_WINDOW_SIZE 5
// Buffer này dành cho 'stop' signal
char cmd_window[CMD_WINDOW_SIZE] = {0};  // Buffer để kiếm tra "STOPP"

int main(void)
{
	// Magic flag giúp báo hiệu đã có flags ở đây
	if(Flash_ReadWord((uint32_t)(&(flags->magic))) != 0xBEFFDEAD)
	{
		Flash_EraseOnePage((uint32_t)ADDR_FLAGS_START);
		Flash_WriteHalfWord((uint32_t)(&(flags->magic)), (uint16_t)0xDEAD);
		Flash_WriteHalfWord((uint32_t)(&(flags->magic)) + 2, (uint16_t)0xBEFF);
		Flash_WriteHalfWord((uint32_t)(&(flags->next_action)), (uint16_t)FLAG_UPDATE_FACTORY);
	}
	switch(flags->next_action)
	{
		case FLAG_UPDATE_FACTORY:
			Initialization();
			UpdateFactoryHandler();
			// Receive factory firmware
			break;
		case FLAG_UPDATE_APP:
			Initialization();
			UpdateApplicationHandler();
			// Receive app firmware
			break;
		case FLAG_RUN_APP:
			JumpToAppFirmware();
			// Run app firmware
			break;
		case FLAG_RUN_FACTORY:
			JumpToFactoryFirmware();
			// Run factory firmware
			break;
		default:
			// Run factory firmware
			break;
	}
	for(;;);
}

void Initialization(void)
{
	// RCC HSI = 8mhz
	RCC_Config_HSI_8MHz();
	// Ringbuffer Init
	Ringbuffer_init(ringbuffer);
	// GPIOC P13 init for toggle signal
	GPIO_init_output(GPIOC, 13);
	// UART GPIO Init for RX (input, pushpull, pullup) and TX
	UART1_gpio_init();

	// SETUP DMA
	// DMA-RX Init
	UART1_DMA_Setup();
	// DMA1 Channel5 init (this channel for uart1 rx)
	DMA1_Channel5_UART1_RX_setup();
}

void UpdateFactoryHandler(void)
{
	UpdateFirmware((uint32_t)ADDR_FACTORY_START, FLAG_RUN_FACTORY);
}

void UpdateApplicationHandler(void)
{
	UpdateFirmware((uint32_t)ADDR_APP_START, FLAG_RUN_APP);
}

void JumpToFactoryFirmware(void)
{
	// Lấy địa chỉ msp của app
	uint32_t factory_msp = *(volatile uint32_t*)(ADDR_FACTORY_START);
	// Lấy địa chỉ resethandler của app
	uint32_t factory_rshandler = *(volatile uint32_t*)(ADDR_FACTORY_START + 4);
	// Set msp hiện hành thành msp của app
	__asm volatile ("msr msp, %0" : : "r" (factory_msp) : );
	// Set lại offset vector table của app
	*(uint32_t*)0xE000ED08 = ADDR_FACTORY_START;
	// Trỏ đến rshander của app
	factoryFuncPointer fac_pointer = (factoryFuncPointer)factory_rshandler;
	fac_pointer();
	while(1);
}

void JumpToAppFirmware(void)
{
	// Lấy địa chỉ msp của app
	uint32_t app_msp = *(volatile uint32_t*)(ADDR_APP_START);
	// Lấy địa chỉ resethandler của app
	uint32_t app_rshandler = *(volatile uint32_t*)(ADDR_APP_START + 4);
	// Set msp hiện hành thành msp của app
	__asm volatile ("msr msp, %0" : : "r" (app_msp) : );
	// Set lại offset vector table của app
	*(uint32_t*)0xE000ED08 = ADDR_APP_START;
	// Trỏ đến rshander của app
	appFuncPointer app_pointer = (factoryFuncPointer)app_rshandler;
	app_pointer();
	while(1);
}

/*
 * @brief: Hàm này có tác dụng, push data theo kiểu từ phải sang trái
 * 		   Nếu full thì đẩy tiếp, data sẽ thêm ở cuối array và loại bỏ data ở đầu array.
 * 		   Sử dụng cái kỹ thuật này vì, nếu dùng max về 0 như nhận start,
 * 		   Sẽ có các lỗi không mong muốn như 'p\0stop'
 * @param: char new_char (ký tự cần thêm vào arr)
 * @ret val: void
 * */
void push_char_to_window(char new_char)
{
    // Đẩy toàn bộ mảng sang trái 1 ký tự (copy sang trái)
    memmove(cmd_window, cmd_window + 1, CMD_WINDOW_SIZE - 1);
    // Gán ký tự mới vào cuối mảng
    cmd_window[CMD_WINDOW_SIZE - 1] = new_char;
}

void UpdateFirmware(uint32_t address, uint16_t next_action_flag)
{
	uint32_t current_flash_address = address;
	uint8_t data;
	uint8_t tmp_data;
	uint8_t has_tmp_data = 0;

	// Receive state
	enum{
		WAIT_START,
		RECEIVING,
		WAIT_STOP
	};
	// Set trạng thái đầu tiên luôn ở Wait_start
	uint8_t state = WAIT_START;
	// Buffer này dành cho 'start' signal
	char command_buff[6] = {0};
	uint8_t cmd_index = 0;
	while(1)
	{
		// Nếu đã nhận được stop, reset ngay, không chờ data nữa
		if(state == WAIT_STOP)
		{
			Flash_EraseOnePage((uint32_t)ADDR_FLAGS_START);
			Flash_WriteHalfWord((uint32_t)(&(flags->magic)), (uint16_t)0xDEAD);
			Flash_WriteHalfWord((uint32_t)(&(flags->magic)) + 2, (uint16_t)0xBEFF);
			Flash_WriteHalfWord((uint32_t)(&(flags->next_action)), (uint16_t)next_action_flag);
			GPIO_toggle_pin(GPIOC, 13);
			// Soft reset, 0x05FA là key, bit 2 là bit reset
			RESET_AIRCR = (0x05FA << 16) | (1 << 2);
		}
		// Update ringbuffer từ DMA
		DMA1_Channel5_update_ringbuffer();
		// Check ringbuffer, buff có data thì thực hiện write
		if(Ringbuffer_get(ringbuffer, &data))
		{
			switch(state)
			{
			case WAIT_START:
				if(cmd_index < sizeof(command_buff) - 1)
				{
					// Toán tử index++, gán data sau đó mới cộng
					command_buff[cmd_index++] = data;
					// Kết thúc bằng \0 để báo hiệu kết thúc chuỗi và so sánh
					command_buff[cmd_index] = '\0';
					if(strstr(command_buff, "START"))
					{
						// Xóa 10 page bắt đầu từ current_flash_address
						for (int i = 0; i < 20; i++)
						{
							Flash_EraseOnePage(current_flash_address + i * 1024);
						}
						cmd_index = 0;
						has_tmp_data = 0;
						state = RECEIVING;
					}
				}
				else
				{
					cmd_index = 0;
				}
				break;
			case RECEIVING:
				// Sử dụng kỹ thuật left pushing technique
				push_char_to_window(data);
				if (strncmp(cmd_window, "STOPP", CMD_WINDOW_SIZE) == 0)
				{
					state = WAIT_STOP;
					break;
				}
				if(!has_tmp_data)	// biến has tmp data để hỗ trợ việc ghép 2 data và tmp_data
				{
					tmp_data = data;
					has_tmp_data = 1;
				}
				else
				{
					// Ghép temp_data và data thành 16bit
					uint16_t halfword = ((uint16_t)data << 8) | tmp_data;
					Flash_WriteHalfWord(current_flash_address, halfword);
					current_flash_address += 2;
					has_tmp_data = 0;
					GPIO_toggle_pin(GPIOC, 13);
				}
				break;
			}
		}
	}
}
