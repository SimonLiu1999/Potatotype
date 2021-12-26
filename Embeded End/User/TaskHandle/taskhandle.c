#include "taskhandle.h"

void send_message_test(){
	uint32_t datalen = strlen("send_message_test\r\n") + 2;
	//Debug("strlen:%d",datalen);
	uint8_t* s = pvPortMalloc(sizeof(char)*datalen);
	s[0] = PRINT;
	sprintf((char*)&s[1], "send_message_test\r\n");
	uint8_t* packet = pack(s,datalen);
	BaseType_t xReturn = pdTRUE;
	xReturn = xQueueSend( QueueToUARTHandle , /* 消息队列的句柄 */
														&packet, /* 接收的消息内容 */
														portMAX_DELAY); 
}

void TaskUARTControl(void *argument){
	for(;;){
		osDelay(1000);
	}
}

void TaskBlinkLED(void *argument)
{
  /* USER CODE BEGIN TaskBlinkLED */
  /* Infinite loop */
  for(;;)
  {	
		//Debug("led\r\n");
		send_message_test();
		HAL_IWDG_Refresh(&hiwdg);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
		osDelay(1);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
		osDelay(1000);
		 
  }
  /* USER CODE END TaskBlinkLED */
}

void TaskProcessor(void *argument)
{
  /* USER CODE BEGIN TaskProcessor */
	uint16_t datalen = 0;
	uint8_t command = 0;
	static char* data;
	
	static BaseType_t xReturn = pdFALSE;
	struct expr_var_list vars = {0};
	struct expr *e; 
	float result;
	short sysclock1 = 0;  
	short sysclock2 = 0;
	
	struct Image image;
  /* Infinite loop */
  for(;;)
  {
		
		xReturn = xQueueReceive( QueueDecoderToProcessorHandle , /* 消息队列的句柄 */
			&data, /* 接收的消息内容 */
			portMAX_DELAY); 
		Debug("in\r\n");
		if (pdTRUE == xReturn){
			datalen = (data[0]<<8) + data[1] - 1; //除掉指令的数据长
			command = data[2];
			Debug("command%d datalen%d\r\n",command,datalen);
			switch(command){
				case TASK_CALCULATION:
					sysclock1 =(short)(TIM7 -> CNT);
					e= expr_create((char*)&data[3], datalen, &vars, NULL);
					if (e == NULL) {
						Debug("Syntax error!\r\n");
						expr_destroy(e,&vars);
					}
					else{
						result = expr_eval(e);
						expr_destroy(e,&vars);
						sysclock2 =(short)(TIM7 -> CNT);
						Debug("result= %.5f  time=%uus\r\n", result,(short)(sysclock2-sysclock1));
					}
					break;
				case TASK_SCREEN:
					image.BlackImage = pvPortMalloc(2888);
					image.YellowImage = pvPortMalloc(2888);
					if(image.BlackImage == NULL)Debug("black fail");
					if(image.YellowImage == NULL)Debug("yellow fail");
					for(int i=0;i<2888;i++){//拷贝图片
						image.BlackImage[i] = data[i+3];
						image.YellowImage[i] = data[i+3+2888];
					}
					xReturn = xQueueSend(QueueProcessorToScreenHandle, &image, 10);
					if (xReturn == pdFALSE){
						vPortFree(image.BlackImage);
						vPortFree(image.YellowImage);
					}
					break;
				default:break;
			}
			vPortFree(data);
		}
	}
  /* USER CODE END TaskProcessor */
}

void TaskUARTDecoder(void *argument)
{
  /* USER CODE BEGIN 5 */
	static BaseType_t xReturn = pdTRUE;
	static uint8_t Rx_queue;
	static uint8_t Status=WAITHEAD1;
	//static uint8_t IfReceiving=NOTRECEIVING;
	static uint16_t DataLenHigh8=0;
	static uint16_t DataLenLow8=0;
	static uint16_t DataLen=0;
	uint16_t i = 0;
	uint8_t CheckSum=0;//校验和
	uint8_t CheckLenSum=0; //数据长度校验和
	char* data;

  /* Infinite loop */
  for(;;)
  {
		xReturn = xQueueReceive( QueueUARTToAllocatorHandle, /* 消息队列的句柄 */
		&Rx_queue, /* 接收的消息内容 */
		(Status==WAITHEAD1?portMAX_DELAY:250)); 
		if (pdTRUE== xReturn){
			//Debug("：0x%x\r\n",Rx_queue);
		}
		else {
			Debug("timeout!0x%lx LENGTH=%d\r\n",xReturn,i);
			if(Status == READDATA || Status == SUMCHECK) {vPortFree(data);}
			Status=WAITHEAD1;//回到初始状态
		}
		switch(Status)//接收状态机
		{
			case WAITHEAD1: if(Rx_queue==HEAD1){Status=WAITHEAD2;} else Status=WAITHEAD1; break;
			case WAITHEAD2: if(Rx_queue==HEAD2)Status=WAITHEAD3; else Status=WAITHEAD1; break;
			case WAITHEAD3: if(Rx_queue==HEAD3){CheckLenSum = 0; Status=GETLEN1; /*	Debug("catch a frame!\r\n");*/}else Status=WAITHEAD1; break;
			case GETLEN1: DataLenHigh8=Rx_queue; CheckLenSum+=DataLenHigh8; Status=GETLEN2; break;
			case GETLEN2: DataLenLow8=Rx_queue; CheckLenSum+=DataLenLow8; DataLen=(DataLenHigh8<<8)+DataLenLow8; Status=LENCHECK; break;
			case LENCHECK: 
				if(Rx_queue == CheckLenSum){//长度校验成功
					i=0; CheckSum=0; //初始化接收data所需数据
					data = pvPortMalloc(sizeof(char) * (DataLen+2));
					data[0] = DataLenHigh8; data[1] = DataLenLow8;
					Status = READDATA;
				}
				else{//长度校验失败，回到初始状态
					Debug("checklen wrong!\r\n");
					Status=WAITHEAD1;
				}
				break;
				//Debug("length=%d\r\n",DataLen);
				//if(Command==TASK_SCREEN){Status=READIMAGE;/*Debug("Taskscreen\r\n");*/}
				//else Status=READDATA;
				//i=0;
				//CheckSum=0;
				//break;

			//case GETCOMMAND: 	 
			//	Command=Rx_queue;
			//	Status=GETLEN1;
			//	break;
			case READDATA: 
				data[i+2]=Rx_queue;
				CheckSum+=Rx_queue;
				i++;
				if(i==DataLen){/*data[i+2]=0;*/Status=SUMCHECK;}
				break; 
			
			//case READIMAGE: 
			//	if(i<2888)BlackImage[i]=Rx_queue;
			//	else YellowImage[i-2888]=Rx_queue;
			//	CheckSum+=Rx_queue;
			//	i++;
			//	if(i==DataLen)Status=SUMCHECK;
			//	break;
				
			case SUMCHECK: 
				//Debug("CheckSum=%d",CheckSum);
				if(Rx_queue==CheckSum){	
					Debug("recieve a frame\r\n");
					xQueueSend(QueueDecoderToProcessorHandle, &data, 10);
					data = NULL;
					//Debug("receive a complete frame\r\n");
					//分配接收到的数据给子任务
					/*switch(Command)
					{
						case TASK_SCREEN:
							xReturn = xSemaphoreGive(ScreenSemaphoreHandle );//给出二值信号量
							if ( xReturn == pdTRUE ) 
								Debug("TaskScreen:\r\n");
							else
							Debug("TaskScreen Block!\r\n");
							break;
							
						case TASK_CALCULATION:
							xReturn = xSemaphoreGive(CalcSemaphoreHandle );//给出二值信号量
							if ( xReturn == pdTRUE ) {
								char* s = pvPortMalloc(sizeof(char)*100);
								sDebug(s, "TaskCalc:\r\n");
								xQueueSend(QueueToUARTHandle , &s, 100);
							}
							else
							Debug("TaskCalc Block!\r\n");
							break;
							
						default: Status=WAITHEAD1;break;
					}	*/
					Status=WAITHEAD1;
				}
				else
				{
					vPortFree(data);
					Debug("checksum wrong!\r\n");
					/*switch(Command)
					{
						case TASK_SCREEN:
							Debug("taskscreen Fail!\r\n");
							break;
						case TASK_CALCULATION:
							Debug("taskCalc Fail!\r\n");
							break;
						default:break;
					}*/
					Status=WAITHEAD1;
				}
				break;
			default: Status=WAITHEAD1;break;
		}
			
  }
  /* USER CODE END 5 */
}

void TaskScreenRefresh(void *argument)
{
  /* USER CODE BEGIN TaskScreenRefresh */
	static BaseType_t xReturn = pdFALSE;
	struct Image image;
  /* Infinite loop */
  for(;;)
  {
		xReturn = xQueueReceive(QueueProcessorToScreenHandle,/* 队列句柄 */
		&image, /*读取元素*/
		portMAX_DELAY); /* 等待时间 */
		if (pdTRUE == xReturn)
		Debug("refreshing screen!\r\n");
		DEV_Module_Init();
		EPD_1IN54C_Init();
    //EPD_1IN54C_Clear();
		//Paint_NewImage(BlackImage, EPD_1IN54C_WIDTH, EPD_1IN54C_HEIGHT, 270, WHITE);
		//Paint_SelectImage(BlackImage);
    //Paint_Clear(WHITE);
		//Paint_SelectImage(YellowImage);
    //Paint_Clear(WHITE);
		//Debug("disp------------------------\r\n");
		EPD_1IN54C_Display(image.BlackImage, image.YellowImage);//刷新屏幕
		//Debug("close 5V, Module enters 0 power consumption ...\r\n");
		Debug("Screen Refresh Finish!\r\n");
    DEV_Module_Exit();
		vPortFree(image.BlackImage);
		vPortFree(image.YellowImage);
  }
  /* USER CODE END TaskScreenRefresh */
}

void UARTTx(void *argument)
{
  /* USER CODE BEGIN UARTTx */
  /* Infinite loop */
  for(;;)
  {
		static char* s;
		BaseType_t xReturn = pdTRUE;
		xReturn = xQueueReceive( QueueToUARTHandle , /* 消息队列的句柄 */
			&s, /* 接收的消息内容 */
			portMAX_DELAY);
		uint32_t datalen = (s[3]<<8)+s[4] + 7;
		//Debug("STRLEN:%d\r\n",datalen);
		if (pdTRUE == xReturn){
			//Debug("%s",s);
			HAL_UART_Transmit(&huart1 , (uint8_t *)s, datalen,100);
			vPortFree(s);
		}
  }
  /* USER CODE END UARTTx */
}

void TaskCalculation(void *argument)
{
  /* USER CODE BEGIN TaskCalculation */
	/*uint16_t error_pos;
	uint8_t compiled_code[256];
	uint16_t compiled_size;
	int32_t val;*/
	//const char *s = "(1+1)+(1*1)";
	static BaseType_t xReturn = pdFALSE;
	struct expr_var_list vars = {0};
	struct expr *e; 
	float result;
	short sysclock1 = 0;  
	short sysclock2 = 0; 
  /* Infinite loop */
  for(;;)
  {
		xReturn = xSemaphoreTake(CalcSemaphoreHandle,/* 二值信号量句柄 */
		portMAX_DELAY); /* 等待时间 */
		if (pdTRUE == xReturn)
		{
			/*Debug("task calc!\r\n");
			compiled_size = parse_exp((char*)DataBuffer, compiled_code, sizeof(compiled_code), &error_pos);
			if (!compiled_size)printf("Parse error at position %u\n", (unsigned)error_pos);
			val = eval_compiled(compiled_code, compiled_size, variable_values);
			printf("result = %d\n", val);*///轻量化实现
			//13K内存实现:
			sysclock1 =(short)(TIM7 -> CNT);
			//e= expr_create((char*)DataBuffer, strlen((char*)DataBuffer), &vars, NULL);
			if (e == NULL) {
				Debug("Syntax error!\r\n");
				expr_destroy(e,&vars);
			}
			else{
				result = expr_eval(e);
				expr_destroy(e,&vars);
				sysclock2 =(short)(TIM7 -> CNT);
				Debug("result= %.5f  time=%uus\r\n", result,(short)(sysclock2-sysclock1));
			}

		}

  }
  /* USER CODE END TaskCalculation */
}

uint8_t* pack(uint8_t* unpacked_data, uint16_t datalen){
	uint8_t* packed_data = pvPortMalloc(datalen + 7);
	uint8_t checksum = 0;
	uint8_t datalenHigh8 = datalen >> 8;
	uint8_t datalenLow8 = datalen & 0x00ff;
	packed_data[0] = HEAD1;
	packed_data[1] = HEAD2;
	packed_data[2] = HEAD3;
	packed_data[3] = datalenHigh8;
	packed_data[4] = datalenLow8;
	packed_data[5] = datalenHigh8 + datalenLow8;
	for(int i=0; i<datalen; i++){
		packed_data[i+6] = unpacked_data[i];
		checksum += unpacked_data[i];
	}
	packed_data[datalen+6] = checksum; //最后一位
	vPortFree(unpacked_data);
	return packed_data;
}
