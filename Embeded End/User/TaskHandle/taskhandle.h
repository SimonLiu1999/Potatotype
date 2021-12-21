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
	TASK_CALCULATION=2
};


#define HEAD1 0x11
#define HEAD2 0x22
#define HEAD3 0x33

uint8_t* pack(uint8_t* unpacked_data, uint16_t datalen);

#endif
