/*
 * runtime.c
 *
 *  Created on: Jun 15, 2025
 *      Author: ACER
 */


#include "runtime.h"
#include "Timer.h"
#include "task.h"

static TaskStatistic task_stats[MAX_TASKS];
volatile uint32_t idleTaskCnt = 0;


void runTimeInit(void)
{
	enable_clock_for_timer1();
	setup_timer1();
}

uint16_t getTime(void)
{
	return tim1getval();
}

void onTaskIn(void)
{
	TaskHandle_t task = xTaskGetCurrentTaskHandle();
	uint16_t now = tim1getval();
	TaskStatistic* stat = getTaskStat(task);
	if(stat != NULL)
	{
		stat->lastTime = now;
	}
}

void onTaskOut(void)
{
	TaskHandle_t task = xTaskGetCurrentTaskHandle();
	uint16_t now = tim1getval();
	TaskStatistic* stat = getTaskStat(task);
	if(stat != NULL)
	{
		uint32_t delta;
		// Xử lý tràn timer
		if(now >= stat->lastTime)
		{
			delta = now - stat->lastTime;
		}
		else
		{
			// Ví dụ last = 65000, count 1000 -> 65536 + 1000 - 65000 = đúng
			// Tránh tràn timer gây ra tính sai lệch
			delta = (uint32_t)(0x10000 + now - stat->lastTime);
		}
		// Tổng thời gian bằng số cũ + số mới
		stat->totalTime += delta;
	}
}

TaskStatistic* getTaskStat(TaskHandle_t task)
{
	// xét cái mảng hiện có, nếu gặp task nào có ID(taskhandle_t) giống với task thì lấy ra
	for(int i = 0; i < MAX_TASKS; i++)
	{
		if(task_stats[i].handle == task)
		{
			return &task_stats[i];
		}
	}
	// Xét cái mảng hiện có, nếu chưa có task này thì thêm nó vào mảng
	for(int i = 0; i < MAX_TASKS; i++)
	{
		if(task_stats[i].handle == NULL)
		{
			task_stats[i].handle = task;
			task_stats[i].lastTime = 0;
			task_stats[i].totalTime = 0;
			return &task_stats[i];
		}
	}
	// Nếu không có task nào giống trong task_stats, và task_stats đã đầy, thì bỏ qua
	return NULL;
}

void CPU_Usage(void)
{
	uint32_t total = 0;
	// Tính tổng thời gian cpu
	for(int i = 0; i < MAX_TASKS; i++)
	{
		if(task_stats[i].handle != NULL)
		{
			total += task_stats[i].totalTime;
			task_stats[i].taskName = *pcTaskGetName(task_stats[i].handle);
		}
	}
	// Tính % từng task
	for(int i = 0; i < MAX_TASKS; i++)
	{
		// Trừ trường hợp total = 0!
		if(task_stats[i].handle != NULL && total > 0)
		{
			if(task_stats[i].totalTime > 1000000)
			{
				task_stats[i].usage = 100;
			}
			else
			{
				task_stats[i].usage = (task_stats[i].totalTime*100) / 1000000;
			}
		}
		else
		{
			task_stats[i].usage = 0;
		}
	}
	// Reset lại mỗi chu kỳ 1s
	for(int i = 0; i < MAX_TASKS; i++)
	{
		task_stats[i].totalTime = 0;
		task_stats[i].lastTime = 0;
	}
}

void vApplicationIdleHook(void)
{
	idleTaskCnt++;
}
