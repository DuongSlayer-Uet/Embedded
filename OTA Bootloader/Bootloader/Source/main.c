/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Ech Xanh UET
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Đây là hàm main của custom mini bootloader
 *
 ******************************************************************************
 */

#include <stdint.h>
#include "gpio.h"
#include "Timer.h"
#include "UART.h"
#include "RCC.h"
#include "Ringbuffer.h"
#include "Interrupt.h"
#include "Flash.h"
#include "DMA.h"

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

static uint8_t is_first_write = 1; // Cờ để kiểm tra lần ghi đầu tiên
static uint32_t current_flash_address = 0x08002000;
// Địa chỉ bắt đầu của application
#define app_start_address 0x08002000
// Con trỏ hàm reset handler của application
typedef void (*appfuncpointer)(void);

void goto_app();

int main(void)
{
	RCC_Config_HSI_8MHz();
	// Khởi tạo ringbuffer
	Ringbuffer_init(ringbuffer);
	// Khởi tạo PC13
	GPIO_init_output(GPIOC, 13);
	// Khởi tạo gpio
	UART1_gpio_init();

	// Setup DMA-RX-UART1
	UART1_DMA_Setup();

	DMA1_Channel5_UART1_RX_setup();

	// SETUP NGẮT BOOTPIN
	// Enable clock for INT pin
	RCC_APB2ENR |= (1 << 2) | (1 << 0);
	// Enable INT5_9 from NVIC
	NVIC_Enable_IRQ(EXTI5_9_IRQ_NUM);
	// Enable Int5 from peripheral, rising edge
	EXTI_Init(GPIOA, 5, 1);

	// Check xem có chương trình chưa
	if(Flash_ReadHalfWord(0x08002400) == 0xFFFF)
	{
		Flash_EraseOnePage(0x0800C800);
		Flash_WriteHalfWord(0x0800C800, 0x0ABC);
	}

	uint8_t data;
	uint8_t tmp_data;
	uint8_t has_tmp_data = 0;
	while(1)
	{
		if(Flash_ReadHalfWord(0x0800C800) == 0x0ABC)	// Check cờ boot mode
		{
			// Update ringbuffer từ DMA
			DMA1_Channel5_update_ringbuffer();
			if(Ringbuffer_get(ringbuffer, &data))	// Check ringbuffer, buff có data thì thực hiện write
			{
				if(is_first_write == 1)		// Trước khi write lần đầu tiên thì erase app cũ đi (3 page = ~3kb)
				{
					// Xóa 3 page (3kb) mỗi kb ~ 0x400
					Flash_EraseOnePage(0x08002000);
					Flash_EraseOnePage(0x08002400);
					Flash_EraseOnePage(0x08002800);
					is_first_write = 0;
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
			}
		}
		else	// Check cờ boot mode
		{
			goto_app();
		}
	}
}

/*
 * @brief Hàm này dùng để set msp, set lại offset vectortable, nhảy đến rshandler của app
 * 		  Thông tin về các địa chỉ thanh ghi này có đề cập trong tài liệu arm cortex M3
 * @param void
 * @retval none
 */
void goto_app()
{
	// Lấy địa chỉ msp của app
	uint32_t app_msp = *(volatile uint32_t*)(app_start_address);
	// Lấy địa chỉ resethandler của app
	uint32_t app_rshandler = *(volatile uint32_t*)(app_start_address + 4);
	// Set msp hiện hành thành msp của app
	__asm volatile ("msr msp, %0" : : "r" (app_msp) : );
	// Set lại offset vector table của app
	*(uint32_t*)0xE000ED08 = app_start_address;
	// Trỏ đến rshander của app
	appfuncpointer app_pointer = (appfuncpointer)app_rshandler;
	app_pointer();
	while(1);
}
