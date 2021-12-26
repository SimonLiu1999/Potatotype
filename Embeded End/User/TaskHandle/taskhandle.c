#include "taskhandle.h"

void send_message_test(){
	uint32_t datalen = strlen("send_message_test\r\n") + 2;
	//Debug("strlen:%d",datalen);
	uint8_t* s = pvPortMalloc(sizeof(char)*datalen);
	s[0] = PRINT;
	sprintf((char*)&s[1], "send_message_test\r\n");
	uint8_t* packet = pack(s,datalen);
	BaseType_t xReturn = pdTRUE;
	xReturn = xQueueSend( QueueToUARTHandle , /* ��Ϣ���еľ�� */
														&packet, /* ���յ���Ϣ���� */
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
		
		xReturn = xQueueReceive( QueueDecoderToProcessorHandle , /* ��Ϣ���еľ�� */
			&data, /* ���յ���Ϣ���� */
			portMAX_DELAY); 
		Debug("in\r\n");
		if (pdTRUE == xReturn){
			datalen = (data[0]<<8) + data[1] - 1; //����ָ������ݳ�
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
					for(int i=0;i<2888;i++){//����ͼƬ
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
	uint8_t CheckSum=0;//У���
	uint8_t CheckLenSum=0; //���ݳ���У���
	char* data;

  /* Infinite loop */
  for(;;)
  {
		xReturn = xQueueReceive( QueueUARTToAllocatorHandle, /* ��Ϣ���еľ�� */
		&Rx_queue, /* ���յ���Ϣ���� */
		(Status==WAITHEAD1?portMAX_DELAY:250)); 
		if (pdTRUE== xReturn){
			//Debug("��0x%x\r\n",Rx_queue);
		}
		else {
			Debug("timeout!0x%lx LENGTH=%d\r\n",xReturn,i);
			if(Status == READDATA || Status == SUMCHECK) {vPortFree(data);}
			Status=WAITHEAD1;//�ص���ʼ״̬
		}
		switch(Status)//����״̬��
		{
			case WAITHEAD1: if(Rx_queue==HEAD1){Status=WAITHEAD2;} else Status=WAITHEAD1; break;
			case WAITHEAD2: if(Rx_queue==HEAD2)Status=WAITHEAD3; else Status=WAITHEAD1; break;
			case WAITHEAD3: if(Rx_queue==HEAD3){CheckLenSum = 0; Status=GETLEN1; /*	Debug("catch a frame!\r\n");*/}else Status=WAITHEAD1; break;
			case GETLEN1: DataLenHigh8=Rx_queue; CheckLenSum+=DataLenHigh8; Status=GETLEN2; break;
			case GETLEN2: DataLenLow8=Rx_queue; CheckLenSum+=DataLenLow8; DataLen=(DataLenHigh8<<8)+DataLenLow8; Status=LENCHECK; break;
			case LENCHECK: 
				if(Rx_queue == CheckLenSum){//����У��ɹ�
					i=0; CheckSum=0; //��ʼ������data��������
					data = pvPortMalloc(sizeof(char) * (DataLen+2));
					data[0] = DataLenHigh8; data[1] = DataLenLow8;
					Status = READDATA;
				}
				else{//����У��ʧ�ܣ��ص���ʼ״̬
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
					//������յ������ݸ�������
					/*switch(Command)
					{
						case TASK_SCREEN:
							xReturn = xSemaphoreGive(ScreenSemaphoreHandle );//������ֵ�ź���
							if ( xReturn == pdTRUE ) 
								Debug("TaskScreen:\r\n");
							else
							Debug("TaskScreen Block!\r\n");
							break;
							
						case TASK_CALCULATION:
							xReturn = xSemaphoreGive(CalcSemaphoreHandle );//������ֵ�ź���
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
		xReturn = xQueueReceive(QueueProcessorToScreenHandle,/* ���о�� */
		&image, /*��ȡԪ��*/
		portMAX_DELAY); /* �ȴ�ʱ�� */
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
		EPD_1IN54C_Display(image.BlackImage, image.YellowImage);//ˢ����Ļ
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
		xReturn = xQueueReceive( QueueToUARTHandle , /* ��Ϣ���еľ�� */
			&s, /* ���յ���Ϣ���� */
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
		xReturn = xSemaphoreTake(CalcSemaphoreHandle,/* ��ֵ�ź������ */
		portMAX_DELAY); /* �ȴ�ʱ�� */
		if (pdTRUE == xReturn)
		{
			/*Debug("task calc!\r\n");
			compiled_size = parse_exp((char*)DataBuffer, compiled_code, sizeof(compiled_code), &error_pos);
			if (!compiled_size)printf("Parse error at position %u\n", (unsigned)error_pos);
			val = eval_compiled(compiled_code, compiled_size, variable_values);
			printf("result = %d\n", val);*///������ʵ��
			//13K�ڴ�ʵ��:
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
	packed_data[datalen+6] = checksum; //���һλ
	vPortFree(unpacked_data);
	return packed_data;
}
