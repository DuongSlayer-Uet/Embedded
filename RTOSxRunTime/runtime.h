/*
 * runtime.h
 *
 *  Created on: Jun 15, 2025
 *      Author: ACER
 */

#ifndef INC_RUNTIME_H_
#define INC_RUNTIME_H_

#include <stdint.h>
#include "FreeRTOS.h"

// forward declare
struct tskTaskControlBlock;
typedef struct tskTaskControlBlock* TaskHandle_t;


#define MAX_TASKS 3
typedef struct
{
	TaskHandle_t handle;
	uint32_t totalTime;
	uint32_t lastTime;
	uint8_t usage;
	uint8_t taskName;
} TaskStatistic;

/*
 * brief: Hàm này khởi tạo TIM1
 * param void
 * retval void
 */
void runTimeInit(void);
/*
 * brief: Hàm này lấy giá trị hiện tại của thanh ghi CNT của TIM1
 * param void
 * retval uint16_t
 */
uint16_t getTime(void);
/*
 * brief: Hàm này được gọi khi có context switching, khi bắt đầu 1 task mới
 * 		  Đã viết define override ở FreeRTOSConfig.h
 * param void
 * retval void
 */
void onTaskIn(void);
/*
 * brief: Hàm này được gọi khi có context switching, Khi kết thúc 1 task
 * 		  Đã viết define override ở FreeRTOSConfig.h
 * param void
 * retval void
 */
void onTaskOut(void);
/*
 * brief: Hàm này dùng để lấy ID (TaskHandle_t) của task đang chạy, từ đó add vào struct của mình
 * param TaskHandle_t (kiểu tự định nghĩa trong task.h) task
 * retval TaskStatistic*
 */
TaskStatistic* getTaskStat(TaskHandle_t task);
/*
 * brief: Hàm này Tính phần trăm CPU đã sử dụng
 * param TaskHandle_t (kiểu tự định nghĩa trong task.h) task
 * retval TaskStatistic*
 */
void CPU_Usage(void);

void vApplicationIdleHook(void);

#endif /* INC_RUNTIME_H_ */
