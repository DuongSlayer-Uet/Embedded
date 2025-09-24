/*
 * BMP180_I2C1.h
 *
 *  Created on: Aug 19, 2025
 *      Author: ACER
 */

#ifndef BMP180_H_
#define BMP180_H_

#include <stdint.h>

typedef struct
{
	volatile uint32_t CR1;
	volatile uint32_t CR2;
	volatile uint32_t OAR1;
	volatile uint32_t OAR2;
	volatile uint32_t DR;
	volatile uint32_t SR1;
	volatile uint32_t SR2;
	volatile uint32_t CCR;
	volatile uint32_t TRISE;
}BMP180_I2C1Typedef_t;

#define BMP180_I2C1		((BMP180_I2C1Typedef_t*)0x40005400)

// SR1 reg
#define BMP180_I2C1_SR1_STOPP	(1 << 4)
#define BMP180_I2C1_SR1_RXNE	(1 << 6)
#define BMP180_I2C1_SR1_SB		(1 << 0)
/*
 * Bit ADDR này là 1 bit ack riêng cho việc check device address
 * Nó được set khi slave nhận về address đúng và có phản hồi lại ack
 * Nó được reset bằng cách đọc SR1, sau đó đọc SR2
 */
#define BMP180_I2C1_SR1_ADDR	(1 << 1)
/*
 * Bit AF (Acknowledge failure) này được set khi có nack từ slave
 */
#define BMP180_I2C1_SR1_AF		(1 << 10)
/*
 * Bit TXE chỉ đảm bảo việc data đã được đẩy hết sang shift reg
 * Chứ không có nghĩa data được gửi đi hoàn toàn và có ACK từ slave
 */
#define BMP180_I2C1_SR1_TXE		(1 << 7)
/*
 * Chú ý: Bit BTF (byte transfer finished) này chỉ được set khi data đã được gửi đi hoàn toàn
 * 		và có ACK trả về từ slave (The BTF bit is not set after a NACK reception - trích nguyên văn datasheet)
 */
#define BMP180_I2C1_SR1_BTF		(1 << 2)

// CR1 reg
#define BMP180_I2C1_CR1_START 	(1 << 8)
#define BMP180_I2C1_CR1_STOP 	(1 << 9)
#define BMP180_I2C1_CR1_ACK		(1 << 10)
#define BMP180_I2C1_CR1_POS		(1 << 11)


/*
 * This func config BMP180_I2C1 as a receiver slave
 * The address of this slave is 0x32
 * ACK is enable
 */
void BMP180_I2C1_SlaveConfig(void);
/*
 * Brief: This func config BMP180_I2C1 as a master
 */
void BMP180_I2C1_MasterConfig(void);

uint8_t BMP180_I2C1_Write8(uint8_t devAddr, uint8_t regAddr, uint8_t regVal);

uint8_t BMP180_I2C1_Read8(uint8_t devAddr, uint8_t regAddr);

uint16_t BMP180_I2C1_Read16(uint8_t devAddr, uint8_t regAddr);

#endif /* BMP180_H_ */
