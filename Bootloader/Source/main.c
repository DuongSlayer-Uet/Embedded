#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "Metadata.h"
#include "Flash.h"
#include "DMA.h"
#include "RCC.h"
#include "Ringbuffer.h"
#include "gpio.h"
#include "IWDG.h"
#include "Timer.h"
#include "Interrupt.h"
#include "CRC.h"
#include "UART.h"
#include "UARTxDMA.h"
#include "DMAxRingBuffer.h"

// System register control - thanh ghi soft reset
#define RESET_AIRCR				(*(volatile uint32_t*)0xE000ED0C)
// CSR - Thanh ghi truy vết reset
#define RCC_CSR					(*(volatile uint32_t*)(0x40021000 + 0x024))
// Max trial boot count
#define MAXTRIALBOOT	5
// SPIxDMA rx buffer
extern uint8_t UARTxDMA_RxBuffer[UARTxDMA_RxBufferSize];
// Ringbuffer
extern RingBuffer_Typedef RingBuffer;

// APP state
typedef struct
{
	uint32_t boot;
	uint32_t ID;
	uint32_t Ver;
	uint32_t Siz;
	uint32_t Crc;
	uint32_t EntryAddr;
	uint32_t ElapsedTime;
} APPInfor_t;

// Metadata
Metadata_t mainMetadata;
// Elapsed time counter (=1ms/count)
volatile uint32_t elapsedTimeCNT = 0;
// Function pointer
typedef void (*app1FuncPointer)(void);
typedef void (*app2FuncPointer)(void);

// Function prototype
void UpdateFirmware(void);
void Initialization(void);
void TIM1_UP_IRQHandler(void);
uint32_t crcCal(uint32_t address, uint32_t size);
void JumpToApp1(void);
void JumpToApp2(void);
void RollBack(void);

int main()
{
	// Init Bootpin
	GPIO_InitBootPin();
	Initialization();
	IWDG_setup();
	RollBack();
	if(GPIO_ReadBootPin() == 0)
	{
		// Update app
		UpdateFirmware();
	}
	else
	{
		//GPIO_DisableBootPin();
		Flash_readMetadata(&mainMetadata);
		if(mainMetadata.activeFirmwareStatus == 1)
		{
			GPIO_set(GPIOA, 8);
			GPIO_reset(GPIOB, 15);
			JumpToApp1();
		}
		else if(mainMetadata.activeFirmwareStatus == 2)
		{
			GPIO_set(GPIOB, 15);
			GPIO_reset(GPIOA, 8);
			JumpToApp2();
		}
		else
		{
			while(1)
			{
				//IWDG_refresh();
			}
		}
	}
}

void Initialization(void)
{
	// RCC HSI = 8mhz
	RCC_Config_HSI_8MHz();
	// Ringbuffer Init
	Ringbuffer_init(&RingBuffer);
	// GPIOC P13 init for toggle signal
	GPIO_init_output(GPIOC, 13);
	// GPIO APP1,2 for APP state
	GPIO_init_output(GPIOA, 8);
	GPIO_init_output(GPIOB, 15);
	// CRC
	CRC_Setup();
	// UART setup
	UART1_init();
	// UARTxDMA config
	UARTxDMA_Config();
	// Timer1 setup
	TIM1_SetUp();
	// TIM1 enable IRQNVIC
	NVIC_Enable_IRQ(25);
	// High speed for flash (two wait states)
	Flash_ConfigHighSpeed();
}

void UpdateFirmware(void)
{
	APPInfor_t app;
	memset(&app, 0, sizeof(app));
	app.Siz = 1;
	uint8_t data;
	uint8_t appInforCNT = 0;
	uint8_t appInfor[20];
	uint8_t byteCount 			= 0;
	uint16_t halfword 			= 0;
	uint32_t crcResult 			= 0;
	uint32_t address		 	= 0;
	uint32_t byteNum = 0;
	elapsedTimeCNT = 0;
	while(1)
	{
		DMAxRingBuffer_UpdateData();

		// Đọc bootpin
		if(byteNum == app.Siz)
		{
			// Send ACK cuối cùng
			UART1_send_data(0xAA);
			// Nếu còn dư 1 byte lẻ
			if(byteCount == 1)
			{
				address += 2;
				halfword |= (0xFF << 8);			// Padding thêm 1 byte 0xFF
				Flash_WriteHalfWord(address, data);	// Ghi nốt vào
				byteCount = 0;
				halfword = 0;
			}
			// Dừng đếm, get counter
			TIM1_GetCNT();
			// Gán thông số nhận được
			app.ElapsedTime = elapsedTimeCNT;
			app.Crc = (appInfor[15] << 24) | (appInfor[14] << 16) | (appInfor[13] << 8) | appInfor[12];
			app.Ver = Flash_encodeVer16bit(appInfor[4], appInfor[5], appInfor[6]);
			app.ID 	= appInfor[7];
			crcResult = crcCal(app.EntryAddr, app.Siz);
			// Check CRC match
			if(crcResult == app.Crc)
			{
				Flash_readMetadata(&mainMetadata);

				// Nếu active là 1, app là 1 => ghi đè
				// Nếu active là 1, app là 2 => ghi thêm 2 và set oldFirmware là 1
				//if(mainMetadata.activeFirmwareStatus == 1)

				// Nếu là app1
				if(mainMetadata.activeFirmwareStatus == 1)
				{
					if(app.ID == 1)
					{
						mainMetadata.activeFirmwareStatus 		= 1;
						mainMetadata.CRCApp1 					= app.Crc;
						mainMetadata.versionApp1 				= app.Ver;
						mainMetadata.sizeApp1 					= app.Siz;
						mainMetadata.app1EntryAddr				= app.EntryAddr;
						mainMetadata.elapsedTime				= app.ElapsedTime;
						mainMetadata.trialBootFlag				= 1;
						mainMetadata.trialBootCount 			= 0;
					}
					else if(app.ID == 2)
					{
						mainMetadata.activeFirmwareStatus 		= 2;
						mainMetadata.oldFirmwareStatus			= 1;					// Set old version
						mainMetadata.CRCApp2 					= app.Crc;
						mainMetadata.versionApp2				= app.Ver;
						mainMetadata.sizeApp2					= app.Siz;
						mainMetadata.app2EntryAddr				= app.EntryAddr;
						mainMetadata.elapsedTime				= app.ElapsedTime;
						mainMetadata.trialBootFlag				= 1;
						mainMetadata.trialBootCount				= 0;
					}
				}
				if(mainMetadata.activeFirmwareStatus == 2)
				{
					if(app.ID == 1)
					{
						mainMetadata.activeFirmwareStatus 		= 1;
						mainMetadata.oldFirmwareStatus			= 2;				// Set old version
						mainMetadata.CRCApp1 					= app.Crc;
						mainMetadata.versionApp1 				= app.Ver;
						mainMetadata.sizeApp1 					= app.Siz;
						mainMetadata.app1EntryAddr				= app.EntryAddr;
						mainMetadata.elapsedTime				= app.ElapsedTime;
						mainMetadata.trialBootFlag				= 1;
						mainMetadata.trialBootCount 			= 0;
					}
					else if(app.ID == 2)
					{
						mainMetadata.activeFirmwareStatus 		= 2;
						mainMetadata.CRCApp2 					= app.Crc;
						mainMetadata.versionApp2				= app.Ver;
						mainMetadata.sizeApp2					= app.Siz;
						mainMetadata.app2EntryAddr				= app.EntryAddr;
						mainMetadata.elapsedTime				= app.ElapsedTime;
						mainMetadata.trialBootFlag				= 1;
						mainMetadata.trialBootCount				= 0;
					}
				}
				if(mainMetadata.activeFirmwareStatus == 0xFFFFFFFF)
				{
					if(app.ID == 1)
					{
						mainMetadata.activeFirmwareStatus 		= 1;
						mainMetadata.CRCApp1 					= app.Crc;
						mainMetadata.versionApp1 				= app.Ver;
						mainMetadata.sizeApp1 					= app.Siz;
						mainMetadata.app1EntryAddr				= app.EntryAddr;
						mainMetadata.elapsedTime				= app.ElapsedTime;
						mainMetadata.trialBootFlag				= 1;
						mainMetadata.trialBootCount 			= 0;
					}
					else if(app.ID == 2)
					{
						mainMetadata.activeFirmwareStatus 		= 2;
						mainMetadata.CRCApp2 					= app.Crc;
						mainMetadata.versionApp2				= app.Ver;
						mainMetadata.sizeApp2					= app.Siz;
						mainMetadata.app2EntryAddr				= app.EntryAddr;
						mainMetadata.elapsedTime				= app.ElapsedTime;
						mainMetadata.trialBootFlag				= 1;
						mainMetadata.trialBootCount				= 0;
					}
				}
				Flash_EraseOnePage(METADATA_START_ADDR);
				Flash_writeMetadata(&mainMetadata);
				// Soft reset, 0x05FA là key, bit 2 là bit reset
				RESET_AIRCR = (0x05FA << 16) | (1 << 2);
			}
		}

		// Nếu có data thì handle
		if(Ringbuffer_get(&RingBuffer, &data))
		{
			IWDG_refresh();
			// 20 byte đầu tiên nhận về app Infor
			if(appInforCNT < 20)
			{
				appInfor[appInforCNT] = data;
				if(appInforCNT == 19)
				{
					// Ghép theo little edian
					address = (appInfor[19] << 24) | (appInfor[18] << 16) | (appInfor[17] << 8) | (appInfor[16]);		// App Address
					app.Siz = (appInfor[11] << 24) | (appInfor[10] << 16) | (appInfor[9] << 8) 	| appInfor[8];
					app.EntryAddr = address;
					// SPI gửi data cực nhanh nên hạn chế xử lý nhiều data ở đây
					TIM1_StartCounting();						// Bắt đầu count data uploading
					Flash_eraseMultiplePage(app.EntryAddr, 20);
				}
				appInforCNT++;
			}
			else
			{
				// receive data ...
				halfword |= (data << (8*byteCount));
				byteCount++;
				if(byteCount == 2)		// Halfword
				{
					Flash_WriteHalfWord(address, halfword);
					address += 2;
					byteCount = 0;
					halfword = 0;
				}
				byteNum++;
				if(byteNum % 512 == 0)
				{
					// Send ACK
					UART1_send_data(0xAA);
				}
			}
		}
	}
}

uint32_t crcCal(uint32_t address, uint32_t size)
{
	uint32_t data;
	uint32_t fullWords = size / 4;
	uint32_t remaining = size % 4;
	uint8_t temp[4];
	// Cal cho các words full byte
    for (uint32_t i = 0; i < fullWords; i++)
    {
        data = Flash_ReadWord(address);
        CRC_WriteWord(data);
        address += 4;
    }
    // Padding cho các byte lẻ
    if(remaining > 0)	// nếu có byte lẻ
    {
        for (uint32_t i = 0; i < remaining; i++)
        {
            temp[i] = Flash_ReadByte(address + i);
        }
        for (uint32_t i = remaining; i < 4; i++)
        {
            temp[i] = 0xFF; // padding
        }
        // Ghép thành word theo little endian
        data = ((uint32_t)temp[3] << 24) |
               ((uint32_t)temp[2] << 16) |
               ((uint32_t)temp[1] << 8)  |
               ((uint32_t)temp[0]);
        CRC_WriteWord(data);
    }
	return CRC_GetResult();
}


void TIM1_UP_IRQHandler(void)
{
	if(TIM1->SR & (1 << 0))
	{
		TIM1->SR &= ~(1 << 0);			// Clear flag
		elapsedTimeCNT++;
	}
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

void RollBack(void)
{
	Flash_readMetadata(&mainMetadata);
	if(mainMetadata.trialBootFlag == 1 && (RCC_CSR & (1 << 29)) != 0)
	{
		if(mainMetadata.trialBootCount < MAXTRIALBOOT)		// Nếu < 5
		{
			mainMetadata.trialBootCount++;				// Tăng số lần chạy thử
			RCC_CSR |= (1 << 24);						// Clear cờ IWDG reset
			Flash_eraseMetadata();
			Flash_writeMetadata(&mainMetadata);
		}
		else		// Nếu chạy thử quá 5 lần
		{
			// Clear cờ IWDG
			RCC_CSR |= (1 << 24);
			// Reset các cờ boot
			mainMetadata.trialBootFlag = 0xFFFFFFFF;
			mainMetadata.trialBootCount = 0xFFFFFFFF;
			mainMetadata.elapsedTime = 0xFFFFFFFF;
			// Nếu failed là app1
			if(mainMetadata.activeFirmwareStatus == 1)
			{
				// Xóa app1
				Flash_eraseMultiplePage(mainMetadata.app1EntryAddr, 20);
				IWDG_refresh();
				// Xóa app1 infor
				mainMetadata.CRCApp1 = 0xFFFFFFFF;
				mainMetadata.app1EntryAddr = 0xFFFFFFFF;
				mainMetadata.sizeApp1 = 0xFFFFFFFF;
				mainMetadata.versionApp1 = 0xFFFFFFFF;
				// nếu có app2
				if(mainMetadata.oldFirmwareStatus == 2)
				{
					mainMetadata.activeFirmwareStatus = 2;
					mainMetadata.oldFirmwareStatus = 0xFFFFFFFF;
				}
				else
				{
					mainMetadata.activeFirmwareStatus = 0xFFFFFFFF;
				}
			}
			// Nếu failed app là app2
			else if(mainMetadata.activeFirmwareStatus == 2)
			{
				// Xóa app2
				Flash_eraseMultiplePage(mainMetadata.app2EntryAddr, 20);
				IWDG_refresh();
				// Xóa app2 infor
				mainMetadata.CRCApp2 = 0xFFFFFFFF;
				mainMetadata.app2EntryAddr = 0xFFFFFFFF;
				mainMetadata.sizeApp2 = 0xFFFFFFFF;
				mainMetadata.versionApp2 = 0xFFFFFFFF;
				// nếu có app1
				if(mainMetadata.oldFirmwareStatus == 1)
				{
					mainMetadata.activeFirmwareStatus = 1;
					mainMetadata.oldFirmwareStatus = 0xFFFFFFFF;
				}
				else
				{
					mainMetadata.activeFirmwareStatus = 0xFFFFFFFF;
				}
			}
			Flash_EraseOnePage(METADATA_START_ADDR);
			Flash_writeMetadata(&mainMetadata);
		}
	}
}
