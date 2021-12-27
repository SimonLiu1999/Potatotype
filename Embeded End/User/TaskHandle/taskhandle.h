#ifndef __TASKHANDLE_H
#define __TASKHANDLE_H

#include "main.h"
#include "cmsis_os.h"

#include "stdio.h"
#include "queue.h"
#include "semphr.h"
#include "GUI_Paint.h"
//#include "EPD_Test.h"
#include "DEV_Config.h"
#include "EPD_1in54c.h"
//#include "parse_eval.c"
#include "expr.h"
#include "taskhandle.h"
#include "Debug.h"

enum Status{
	WAITHEAD1=0,
	WAITHEAD2,
	WAITHEAD3,
	GETLEN1,
	GETLEN2,
	LENCHECK,
	READDATA,
	SUMCHECK,
};

enum Command{
	TASK_SCREEN=1,
	TASK_CALCULATION=2,
	ACK=3,
	PRINT=4,
	LOGIN=5,
	BROADCAST=6,
	LOGOUT=7
};


#define HEAD1 0x11
#define HEAD2 0x22
#define HEAD3 0x33

uint8_t* pack(uint8_t* unpacked_data, uint16_t datalen);

extern IWDG_HandleTypeDef hiwdg;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;

extern osMessageQueueId_t queue_from_processorHandle;
extern osMessageQueueId_t queue_to_processorHandle;
extern osMessageQueueId_t queue_to_uartHandle;
extern osMessageQueueId_t queue_from_uartHandle;
extern osMessageQueueId_t queue_eventHandle;
extern osMessageQueueId_t QueueProcessorToScreenHandle;
extern osMessageQueueId_t QueueDecoderToProcessorHandle;
extern osMessageQueueId_t QueueToUARTHandle;
extern osMessageQueueId_t QueueUARTToAllocatorHandle;
extern osSemaphoreId_t CalcSemaphoreHandle;

#endif
