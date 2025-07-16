/*
- Dual-bank firmware architecture
- OTA update capability
- Watchdog-protected trial boot
- Automatic rollback to previous firmware on failure
 */

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
// This reg help find the reason of reseting
#define RCC_CSR					(*(volatile uint32_t*)(0x40021000 + 0x024))
// Variables
#define size 5
char stopBuff[5] = {0};
// Function Pointer
typedef void (*app1FuncPointer)(void);
typedef void (*app2FuncPointer)(void);
// Func prototype declaration
void Initialization(void);
void pushCharRightToLeft(char arr[], char c);
void UpdateFirmware(uint32_t address, uint16_t current_app, uint16_t previous_app);
void JumpToApp1(void);
void JumpToApp2(void);



int main(void)
{
	// Signal Led
	GPIO_init_output(GPIOC, 13);
	// BootPin
	GPIO_InitBootPin();
	// Bật Watchdog
	IWDG_setup();
	// Đọc current flags
	uint16_t current_flag = Flash_ReadHalfWord(METADATA_FLAGS_ADDR);
	// Đọc old flags
	uint16_t old_flag = Flash_ReadHalfWord(METADATA_FLAGS_ADDR + 2);
	// Check chạy lần đầu tiên
	if(current_flag == 0xFFFF)
	{
		Flash_WriteHalfWord(METADATA_FLAGS_ADDR, FIRSTRUN_FLAG);
	}

	////////////////////////////////////////////////
	// Check chống treo chương trình mới với IWDG//
	// Đọc pending update flag (đây là 1 cờ được bật lên khi chương trình mới vừa nạp vào)
	uint16_t retryCount = Flash_ReadHalfWord(METADATA_COUNTING_ADDR);
	uint16_t updatePending = Flash_ReadHalfWord(METADATA_PENDING_ADDR);
	if(updatePending == 1 && (RCC_CSR & (1 << 29)) != 0)
	{
		if(retryCount == 0xFFFF)
		{
			Flash_EraseOnePage(METADATA_COUNTING_ADDR);
			Flash_WriteHalfWord(METADATA_COUNTING_ADDR, 0);
			RCC_CSR |= (1 << 24);		// Clear cờ IWDGRST
		}
		if(retryCount < 5)
		{
			retryCount++;
			Flash_EraseOnePage(METADATA_COUNTING_ADDR);
			Flash_WriteHalfWord(METADATA_COUNTING_ADDR, retryCount);
			RCC_CSR |= (1 << 24);			// Clear cờ IWDGRST
		}
		else if(retryCount == 5)			// Đếm đủ 5 lần try, hết 5 lần thì coi như firmware lỗi
		{
			// Xóa pending flag
			Flash_EraseOnePage(METADATA_PENDING_ADDR);
			Flash_WriteHalfWord(METADATA_PENDING_ADDR, 0);
			// Reset count
			Flash_EraseOnePage(METADATA_COUNTING_ADDR);
			RCC_CSR |= (1 << 24);			// Clear cờ IWDGRST

			////////////////////////////////////////////////////////////////////////
			/////		Roll back			///////////////////////////////////////
			///// Roll back xảy ra khi retryCount new firmware vượt quá 5 lần/////
			///// Chỉ thực hiện rollback khi có đủ 2 firmware trên hệ thống//////
			if(old_flag == APP1_OLD)		// Nếu app cũ là app1
			{
				// roll back về app1
				// set app1 là active app
				// xóa chương trình firmware lỗi (app2)
				// set lại cờ app1_old là 0xFFFF
				Flash_EraseOnePage(METADATA_FLAGS_ADDR);
				Flash_WriteHalfWord(METADATA_FLAGS_ADDR, APP1_ACTIVE);		// Roll back về app1
				Flash_WriteHalfWord(METADATA_FLAGS_ADDR + 2, NONE_OLD_VER);
				for (int i = 0; i < 20; i++)		// Xóa firmware lỗi (app2)
				{
					Flash_EraseOnePage(APP2_START_ADDR + i * 1024);
				}
				// Soft reset, 0x05FA là key, bit 2 là bit reset
				RESET_AIRCR = (0x05FA << 16) | (1 << 2);
			}
			else if(old_flag == APP2_OLD)	// Nếu app cũ là app2
			{
				// roll back về app2
				// set app2 là active app
				// xóa chương trình firmware lỗi (app2)
				// set lại cờ app2_old là 0xFFFF
				Flash_EraseOnePage(METADATA_FLAGS_ADDR);
				Flash_WriteHalfWord(METADATA_FLAGS_ADDR, APP2_ACTIVE);		// Roll back về app2
				Flash_WriteHalfWord(METADATA_FLAGS_ADDR + 2, NONE_OLD_VER);
				for (int i = 0; i < 20; i++)		// Xóa firmware lỗi (app1)
				{
					Flash_EraseOnePage(APP1_START_ADDR + i * 1024);
				}
				// Soft reset, 0x05FA là key, bit 2 là bit reset
				RESET_AIRCR = (0x05FA << 16) | (1 << 2);
				// roll back về app2
			}
			else if(old_flag == NONE_OLD_VER)
			{
				while(1);					// Do nothing
			}
		}
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
				UpdateFirmware(APP2_START_ADDR, APP2_ACTIVE, APP1_OLD);
				break;
			case APP2_ACTIVE:
				// Update app1
				// Ok => set app1_active
				// Soft reset
				Initialization();
				UpdateFirmware(APP1_START_ADDR, APP1_ACTIVE, APP2_OLD);
				break;
			case FIRSTRUN_FLAG:
				// Update app1
				// Ok => set app1_active
				// soft reset
				Initialization();
				UpdateFirmware(APP1_START_ADDR, APP1_ACTIVE, NONE_OLD_VER);
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
	// IWDG Setup

}

void UpdateFirmware(uint32_t address, uint16_t current_app, uint16_t previous_app)
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
	char startBuff[6] = {0};
	uint8_t startCharIndex = 0;
	while(1)
	{
		// Nếu đã nhận được stop, reset ngay, không chờ data nữa
		if(state == WAIT_STOP)
		{
			Flash_EraseOnePage(METADATA_FLAGS_ADDR);
			Flash_WriteHalfWord(METADATA_FLAGS_ADDR, current_app);
			Flash_WriteHalfWord(METADATA_FLAGS_ADDR + 2, previous_app);			// Ghi tại vị trí flags+2 cờ old app hiện tại
			// Phiên chạy thử
			Flash_EraseOnePage(METADATA_PENDING_ADDR);
			Flash_WriteHalfWord(METADATA_PENDING_ADDR, UPDATEPENDING);
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
				if(startCharIndex < sizeof(startBuff) - 1)
				{
					// Toán tử index++, gán data sau đó mới cộng
					startBuff[startCharIndex++] = data;
					// Kết thúc bằng \0 để báo hiệu kết thúc chuỗi và so sánh
					startBuff[startCharIndex] = '\0';
					if(strstr(startBuff, "START"))
					{
						// Xóa 20 page bắt đầu từ current_flash_address
						for (int i = 0; i < 20; i++)
						{
							Flash_EraseOnePage(current_flash_address + i * 1024);
						}
						startCharIndex = 0;
						has_tmp_data = 0;
						state = RECEIVING;
					}
				}
				else
				{
					startCharIndex = 0;
				}
				break;
			case RECEIVING:
				// Refresh Watchdog
				IWDG_refresh();
				// Sử dụng kỹ thuật left pushing technique
				pushCharRightToLeft(stopBuff, data);
				if (strncmp(stopBuff, "STOPP", 5) == 0)
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

void pushCharRightToLeft(char arr[], char c)
{
    for(int i = 0; i < size - 1; i++)
    {
        arr[i] = arr[i + 1];
    }
    arr[size - 1] = c;
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
