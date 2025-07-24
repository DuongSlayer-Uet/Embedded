#include <stdint.h>
#include <string.h>
#include "Metadata.h"
#include "Flash.h"
#include "DMA.h"
#include "UART.h"
#include "RCC.h"
#include "Ringbuffer.h"
#include "gpio.h"
#include "IWDG.h"
#include "Timer.h"
#include "Interrupt.h"

// System register control - thanh ghi soft reset
#define RESET_AIRCR				(*(volatile uint32_t*)0xE000ED0C)
// CSR - Thanh ghi truy vết reset
#define RCC_CSR					(*(volatile uint32_t*)(0x40021000 + 0x024))
// STOP buff size
#define STOPBUFFSIZE			5
// START buff size
#define STARTBUFFSIZE			5
// TX Log ISR buffer
char logBuff[1024] = {0};
// TX Log buff size
volatile uint16_t logSize = 0;
// TX Log index
volatile uint16_t logIndex = 0;
// start buffer
char startBuff[STARTBUFFSIZE] = {0};
// stop buffer
char stopBuff[STOPBUFFSIZE] = {0};
// Function pointer
typedef void (*app1FuncPointer)(void);
typedef void (*app2FuncPointer)(void);
// enum for receive state
typedef enum
{
	WAIT_START,
	RECEIVING,
	WAIT_STOP
} receiveState_t;
// gán trạng thái đầu tiên là chờ bắt đầu
receiveState_t	receiveState = WAIT_START;

// function prototype
void Initialization(void);
void updateFirmware(uint32_t address, uint32_t current_app, uint32_t previous_app);
void pushCharRightToLeft(char arr[], char c);
void JumpToApp1(void);
void JumpToApp2(void);
void rollBack(void);
void UART_Int_Log(char* data, uint16_t size);

int main(void)
{
	// peripherals init
	Initialization();
	// rollback !!!
	rollBack();
	Metadata_t mainMetadata;
	Flash_readMetadata(&mainMetadata);
	// First run checking
	if(Flash_ReadHalfWord((uint32_t)&metadata->activeFirmwareStatus) == 0xFFFF)
	{
		Flash_WriteHalfWord((uint32_t)&metadata->activeFirmwareStatus, FIRST_RUN);
	}
	// nếu Pin A0 == 0, update
	if(GPIO_ReadBootPin() == 0)
	{
		UART_Int_Log("[STM32] Boot Mode\n", strlen("[STM32] Boot Mode\n"));
		switch(Flash_ReadHalfWord((uint32_t)&metadata->activeFirmwareStatus))
		{
		case APP1_ACTIVE:
			UART_Int_Log("[STM32] APP1_ACTIVE\n", strlen("[STM32] APP1_ACTIVE\n"));
			UART_Int_Log("[STM32] APP2_OLD\n", strlen("[STM32] APP2_OLD\n"));
			UART_Int_Log("[STM32] Gonna update APP2\n", strlen("[STM32] Gonna update APP2\n"));
			updateFirmware(APP2_START_ADDR, APP2_ACTIVE, APP1_OLD);
			break;
		case APP2_ACTIVE:
			UART_Int_Log("[STM32] APP2_ACTIVE\n", strlen("[STM32] APP2_ACTIVE\n"));
			UART_Int_Log("[STM32] APP1_OLD\n", strlen("[STM32] APP1_OLD\n"));
			UART_Int_Log("[STM32] Gonna update APP1\n", strlen("[STM32] Gonna update APP1\n"));
			updateFirmware(APP1_START_ADDR, APP1_ACTIVE, APP2_OLD);
			break;
		case FIRST_RUN:
			UART_Int_Log("[STM32] First RUN!\n", strlen("[STM32] First RUN!\n"));
			updateFirmware(APP1_START_ADDR, APP1_ACTIVE, NO_OLD_VER);
			break;
		default:
			break;
		}
	}
	else
	{
		switch(Flash_ReadHalfWord((uint32_t)&metadata->activeFirmwareStatus))
		{
		case APP1_ACTIVE:
			// UART_Int_Log("[STM32] Jump to APP1!\n", strlen("[STM32] Jump to APP1!\n"));
			UART_Log("[STM32] Jump to APP1!\n");
			JumpToApp1();
			break;
		case APP2_ACTIVE:
			// UART_Int_Log("[STM32] Jump to APP2!\n", strlen("[STM32] Jump to APP2!\n"));
			UART_Log("[STM32] Jump to APP2!\n");
			JumpToApp2();
			break;
		case FIRST_RUN:
			while(1)
			{
				IWDG_refresh();
				delay_ms(200);
			}
			break;
		default:
			break;
		}
	}
    // Loop
	for(;;);
}

void rollBack(void)
{
	Metadata_t rollbackMetadata;
	Flash_readMetadata(&rollbackMetadata);
	if(rollbackMetadata.trialBootFlag == 1 && (RCC_CSR & (1 << 29)) != 0)
	{
		if(rollbackMetadata.trialBootCount < MAX_TRIAL_BOOT)
		{
			UART_Log("[STM32] Lỗi Firmware, retry\n");
			rollbackMetadata.trialBootCount = rollbackMetadata.trialBootCount + 1;
			Flash_eraseMetadata();
			Flash_writeMetadata(&rollbackMetadata);
			RCC_CSR |= (1 << 24);
		}
		else
		{
			UART_Log("[STM32] ROLL BACK!\n");
			rollbackMetadata.trialBootCount = 0xFFFF;
			rollbackMetadata.trialBootFlag = TRIAL_MODE_STOP;
			if(rollbackMetadata.oldFirmwareStatus == APP1_OLD)
			{
				rollbackMetadata.activeFirmwareStatus = APP1_ACTIVE;
				rollbackMetadata.oldFirmwareStatus = NO_OLD_VER;
				Flash_eraseMultiplePage(APP2_START_ADDR, 20);
			}
			else if(rollbackMetadata.oldFirmwareStatus == APP2_OLD)
			{
				rollbackMetadata.activeFirmwareStatus = APP2_ACTIVE;
				rollbackMetadata.oldFirmwareStatus = NO_OLD_VER;
				Flash_eraseMultiplePage(APP1_START_ADDR, 20);
			}
			else if(rollbackMetadata.oldFirmwareStatus == NO_OLD_VER)
			{
				rollbackMetadata.activeFirmwareStatus = FIRST_RUN;
				rollbackMetadata.oldFirmwareStatus = NO_OLD_VER;
				Flash_eraseMultiplePage(APP1_START_ADDR, 20);
			}
			// 1 case nữa trong trường hợp chỉ có 1 firmware, và firmware đó hỏng
			Flash_eraseMetadata();
			Flash_writeMetadata(&rollbackMetadata);
			RCC_CSR |= (1 << 24);
		}
	}
}

void Initialization(void)
{
	// RCC HSI = 8mhz
	RCC_Config_HSI_8MHz();
	// Ringbuffer Init
	Ringbuffer_init(ringbuffer);
	// GPIOC P13 init for toggle signal
	GPIO_init_output(GPIOC, 13);
	// Init Bootpin
	GPIO_InitBootPin();
	// UART GPIO Init for RX (input, pushpull, pullup) and TX
	UART1_init();
	// Enable IRQ 37 (usart1 - TX ỉnterrupt) from NVIC
	NVIC_Enable_IRQ(37);
	// DMA1 Channel5 init (this channel for uart1 rx)
	DMA1_Channel5_UART1_RX_setup();
	// DMA1 channel4 init (this channel for uart1 tx)
	DMA1_Channel4_UART1_TX_Interrupt_setup();
	// Bật Watchdog
	// IWDG_setup();
	// Timer
	setup_timer1();
}

void updateFirmware(uint32_t address, uint32_t current_app, uint32_t previous_app)
{
	uint8_t data;
	uint8_t tmp_data;
	uint8_t has_tmp_data = 0;
	uint8_t startIndex = 0;
	while(1)
	{
		// watchdog refresh
		IWDG_refresh();
		// Nếu đã nhận được stop, reset ngay, không chờ data nữa
		if(receiveState == WAIT_STOP)
		{
			Metadata_t metadata_local;
			Flash_eraseMetadata();

			metadata_local.activeFirmwareStatus 	= current_app;
			metadata_local.oldFirmwareStatus		= previous_app;
			metadata_local.trialBootFlag			= TRIAL_MODE_START;
			metadata_local.trialBootCount			= 0;

			Flash_writeMetadata(&metadata_local);
			// Soft reset, 0x05FA là key, bit 2 là bit reset
			RESET_AIRCR = (0x05FA << 16) | (1 << 2);
		}
		// Update ringbuffer từ DMA
		DMA1_Channel5_update_ringbuffer();
		// ringbuffer có data thì write
		if(Ringbuffer_get(ringbuffer, &data))
		{
			switch(receiveState)
			{
			case WAIT_START:
				IWDG_refresh();
				startBuff[startIndex++] = data;
				if (strncmp(startBuff, "START", 5) == 0)
				{
					receiveState = RECEIVING;
					UART_Int_Log("[STM32] START OK!\n", strlen("[STM32] START OK!\n"));
					Flash_eraseMultiplePage(address, 20);
					UART_Int_Log("[STM32] Receiving\n", strlen("[STM32] Receiving\n"));
				}
				break;
			case RECEIVING:
				IWDG_refresh();
				pushCharRightToLeft(stopBuff, data);
				if (strncmp(stopBuff, "STOPP", 5) == 0)
				{
					UART_Log("[STM32] STOP OK!\n");
					UART_Log("[STM32] Received Successfully\n");
					receiveState = WAIT_STOP;
					break;
				}
				// biến has tmp data để hỗ trợ việc ghép 2 data và tmp_data
				if(!has_tmp_data)
				{
					tmp_data = data;
					has_tmp_data = 1;
				}
				else
				{
					// Ghép temp_data và data thành 16bit
					uint16_t halfword = ((uint16_t)data << 8) | tmp_data;
					Flash_WriteHalfWord(address, halfword);
					address += 2;
					has_tmp_data = 0;
					GPIO_toggle_pin(GPIOC, 13);
				}
				break;
			default:
				break;
			}
		}
	}
}

void pushCharRightToLeft(char arr[], char c)
{
    for(int i = 0; i < STOPBUFFSIZE - 1; i++)
    {
        arr[i] = arr[i + 1];
    }
    arr[STOPBUFFSIZE - 1] = c;
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

void UART_Int_Log(char* data, uint16_t size)
{
	// Lưu data vào buff tạm
	for(uint8_t i = 0; i < size; i++)
	{
		logBuff[i] = data[i];
	}
	logSize = size;
	logIndex = 1;
	// Kích chạy lần đầu
	UART1->DR = logBuff[0];
	// Set bit TX interrupt enable
	UART1->CR1 |= TXEIE;
}
