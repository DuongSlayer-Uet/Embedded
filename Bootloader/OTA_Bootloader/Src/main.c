#include <stdint.h>
#include <string.h>
#include "Flash.h"
#include "DMA.h"
#include "UART.h"
#include "RCC.h"
#include "Ringbuffer.h"
#include "gpio.h"
#include "Bootflags.h"
#include "IWDG.h"

// System register control (Dùng để reset)
#define RESET_AIRCR				(*(volatile uint32_t*)0xE000ED0C)
// Variables
char cmd_window[5] = {0};
// Function Pointer
typedef void (*app1FuncPointer)(void);
typedef void (*app2FuncPointer)(void);
// Func prototype declaration
void Initialization(void);
void push_char_to_window(char new_char);
void UpdateFirmware(uint32_t address, uint16_t app_status);
void JumpToApp1(void);
void JumpToApp2(void);



int main(void)
{
	// Signal Led
	GPIO_init_output(GPIOC, 13);
	// BootPin
	GPIO_InitBootPin();

	uint16_t current_flag = Flash_ReadHalfWord(METADATA_FLAGS_ADDR);

	if(current_flag == 0xFFFF)
	{
		Flash_WriteHalfWord(METADATA_FLAGS_ADDR, FIRSTRUN_FLAG);
	}

	// nếu Pin A0 == 0, update
	if(GPIO_ReadBootPin() == 0)
	{
		switch(current_flag)
		{
			case APP1_ACTIVE:
				// Update app2
				// Ok => set app2_active
				// Soft reset
				Initialization();
				UpdateFirmware(APP2_START_ADDR, APP2_ACTIVE);
				break;
			case APP2_ACTIVE:
				// Update app1
				// Ok => set app1_active
				// Soft reset
				Initialization();
				UpdateFirmware(APP1_START_ADDR, APP1_ACTIVE);
				break;
			case FIRSTRUN_FLAG:
				// Update app1
				// Ok => set app1_active
				// soft reset
				Initialization();
				UpdateFirmware(APP1_START_ADDR, APP1_ACTIVE);
				break;
			default:
				break;
		}
	}
	else		// Else case: Run
	{
		switch(current_flag)
		{
			case APP1_ACTIVE:
				JumpToApp1();
				// Run app1
				break;
			case APP2_ACTIVE:
				JumpToApp2();
				// Run app2
				break;
			case FIRSTRUN_FLAG:
				// wait
				while(1);
			default:
				break;
		}
	}

    /* Loop forever */
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

void UpdateFirmware(uint32_t address, uint16_t app_status)
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
			Flash_EraseOnePage(METADATA_FLAGS_ADDR);
			Flash_WriteHalfWord(METADATA_FLAGS_ADDR, app_status);
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
						// Xóa 20 page bắt đầu từ current_flash_address
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
				// Refresh Watchdog
				IWDG_refresh();
				// Sử dụng kỹ thuật left pushing technique
				push_char_to_window(data);
				if (strncmp(cmd_window, "STOPP", 5) == 0)
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

void push_char_to_window(char new_char)
{
    // Đẩy toàn bộ mảng sang trái 1 ký tự (copy sang trái)
    memmove(cmd_window, cmd_window + 1, 5 - 1);
    // Gán ký tự mới vào cuối mảng
    cmd_window[5 - 1] = new_char;
}

void JumpToApp1(void)
{
	// Lấy địa chỉ msp của app
	uint32_t app1_msp = *(volatile uint32_t*)(APP1_START_ADDR);
	// Lấy địa chỉ resethandler của app
	uint32_t app1_rshandler = *(volatile uint32_t*)(APP1_START_ADDR + 4);
	// Set msp hiện hành thành msp của app
	__asm volatile ("msr msp, %0" : : "r" (app1_msp) : );
	// Set lại offset vector table của app
	*(uint32_t*)0xE000ED08 = APP1_START_ADDR;
	// Trỏ đến rshander của app
	app1FuncPointer app1_pointer = (app1FuncPointer)app1_rshandler;
	app1_pointer();
	while(1);
}

void JumpToApp2(void)
{
	// Lấy địa chỉ msp của app
	uint32_t app2_msp = *(volatile uint32_t*)(APP2_START_ADDR);
	// Lấy địa chỉ resethandler của app
	uint32_t app2_rshandler = *(volatile uint32_t*)(APP2_START_ADDR + 4);
	// Set msp hiện hành thành msp của app
	__asm volatile ("msr msp, %0" : : "r" (app2_msp) : );
	// Set lại offset vector table của app
	*(uint32_t*)0xE000ED08 = APP2_START_ADDR;
	// Trỏ đến rshander của app
	app2FuncPointer app2_pointer = (app2FuncPointer)app2_rshandler;
	app2_pointer();
	while(1);
}
