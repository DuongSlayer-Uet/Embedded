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
#include "SPI.h"
#include "SPIxDMA.h"
#include "DMAxRingBuffer.h"

// System register control - thanh ghi soft reset
#define RESET_AIRCR				(*(volatile uint32_t*)0xE000ED0C)
// CSR - Thanh ghi truy vết reset
#define RCC_CSR					(*(volatile uint32_t*)(0x40021000 + 0x024))
// SPIxDMA rx buffer
extern uint8_t SPIxDMA_RxBuffer[SPIxDMA_RXBUFFSIZE];
// Ringbuffer
extern RingBuffer_Typedef RingBuffer;

// Receiving stage
typedef enum
{
	START,
	APPID,
	VERSION,
	SIZE,
	CRC32,
	ENTRYADDR,
	DATA,
	CRCCHECKING
} ReceiveState_t;

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
uint8_t ascii2num(uint8_t c);
uint32_t crcCal(uint32_t address, uint32_t size);

int main()
{
	// Init Bootpin
	GPIO_InitBootPin();
	Flash_readMetadata(&mainMetadata);
	if(GPIO_ReadBootPin() == 0)
	{
		Initialization();
		UpdateFirmware();
		// Update app
	}
	else
	{
		GPIO_DisableBootPin();
		Flash_readMetadata(&mainMetadata);
		if(mainMetadata.activeFirmwareStatus == 1)
		{
			JumpToApp1();
		}
		else if(mainMetadata.activeFirmwareStatus == 2)
		{
			JumpToApp2();
		}
		else
		{
			while(1);
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
	// CRC
	CRC_Setup();
	// SPI x DMA setup
	SPI1_SlaveSetup();
	SPI1xDMA_SlaveSetup();
	// Timer1 setup
	TIM1_SetUp();
	// TIM1 enable IRQNVIC
	NVIC_Enable_IRQ(25);
}

void UpdateFirmware(void)
{
	APPInfor_t app;
	memset(&app, 0, sizeof(app));
	ReceiveState_t ReceiveState	= START;
	uint8_t data;
	uint8_t appInforCNT = 0;
	uint8_t appInfor[20];
	uint8_t byteCount 			= 0;
	uint32_t temp 				= 0;
	uint8_t	patch 				= 0;
	uint8_t minor 				= 0;
	uint8_t major 				= 0;
	uint16_t halfword 			= 0;
	uint32_t crcResult 			= 0;
	uint32_t address		 	= 0;
	while(1)
	{
		DMAxRingBuffer_UpdateData();
		// Đọc pin chip select
		if((GPIOA->IDR & (1 << 4)) != 0)
		{
			// Dừng đếm, get counter
			TIM1_GetCNT();
			// convert sang số thường
			// Do có vài số mình cần ở dạng hex, vài số cần ở dạng dec, nên cần convert vài số thui =))
			for(int i = 5; i <= 7; i++)
			{
				appInfor[i] = ascii2num(appInfor[i]);
			}
			// Gán thông số nhận được
			app.ElapsedTime = elapsedTimeCNT;
			app.Crc = (appInfor[15] << 24) | (appInfor[14] << 16) | (appInfor[13] << 8) | appInfor[12];
			app.Siz = (appInfor[11] << 24) | (appInfor[10] << 16) | (appInfor[9] << 8) 	| appInfor[8];
			app.Ver = Flash_encodeVer16bit(appInfor[5], appInfor[6], appInfor[7]);
			app.ID 	= appInfor[4];
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
					if(app.ID == '1')
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
					else if(app.ID == '2')
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
					if(app.ID == '1')
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
					else if(app.ID == '2')
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
					if(app.ID == '1')
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
					else if(app.ID == '2')
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
			// 20 byte đầu tiên nhận về app Infor
			if(appInforCNT < 20)
			{
				appInfor[appInforCNT] = data;
				appInforCNT++;
				if(appInforCNT == 19)
				{
					// Ghép theo little edian
					address = (appInfor[19] << 24) | (appInfor[18] << 16) | (appInfor[17] << 8) | (appInfor[16]);		// App Address
					app.EntryAddr = address;
					// SPI gửi data cực nhanh nên hạn chế xử lý nhiều data ở đây
					TIM1_StartCounting();		// Bắt đầu count data uploading
				}
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

uint8_t ascii2num(uint8_t c)
{
    return c - '0';   // '0' = 48, '1' = 49, ...
}
