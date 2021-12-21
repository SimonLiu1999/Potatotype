/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
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


//#define RECEIVING 1
//#define NOTRECEIVING 0

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
IWDG_HandleTypeDef hiwdg;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim7;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;

/* Definitions for UARTDecoder */
osThreadId_t UARTDecoderHandle;
const osThreadAttr_t UARTDecoder_attributes = {
  .name = "UARTDecoder",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for ScreenRefresh */
osThreadId_t ScreenRefreshHandle;
const osThreadAttr_t ScreenRefresh_attributes = {
  .name = "ScreenRefresh",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for Calculation */
osThreadId_t CalculationHandle;
const osThreadAttr_t Calculation_attributes = {
  .name = "Calculation",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for BlinkLED */
osThreadId_t BlinkLEDHandle;
const osThreadAttr_t BlinkLED_attributes = {
  .name = "BlinkLED",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityRealtime,
};
/* Definitions for uarttx */
osThreadId_t uarttxHandle;
const osThreadAttr_t uarttx_attributes = {
  .name = "uarttx",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for processor */
osThreadId_t processorHandle;
const osThreadAttr_t processor_attributes = {
  .name = "processor",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for QueueUARTToAllocator */
osMessageQueueId_t QueueUARTToAllocatorHandle;
const osMessageQueueAttr_t QueueUARTToAllocator_attributes = {
  .name = "QueueUARTToAllocator"
};
/* Definitions for QueueToUART */
osMessageQueueId_t QueueToUARTHandle;
const osMessageQueueAttr_t QueueToUART_attributes = {
  .name = "QueueToUART"
};
/* Definitions for QueueDecoderToProcessor */
osMessageQueueId_t QueueDecoderToProcessorHandle;
const osMessageQueueAttr_t QueueDecoderToProcessor_attributes = {
  .name = "QueueDecoderToProcessor"
};
/* Definitions for QueueProcessorToScreen */
osMessageQueueId_t QueueProcessorToScreenHandle;
const osMessageQueueAttr_t QueueProcessorToScreen_attributes = {
  .name = "QueueProcessorToScreen"
};
/* Definitions for CalcSemaphore */
osSemaphoreId_t CalcSemaphoreHandle;
const osSemaphoreAttr_t CalcSemaphore_attributes = {
  .name = "CalcSemaphore"
};
/* USER CODE BEGIN PV */
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif
PUTCHAR_PROTOTYPE
{
    //??????????huart3?????
    HAL_UART_Transmit(&huart3 , (uint8_t *)&ch, 1,100);
    return ch;
}

uint8_t UpperRx=0;
//UBYTE BlackImage[2888];
//UBYTE YellowImage[2888];
//UWORD Imagesize = ((EPD_1IN54C_WIDTH % 8 == 0)? (EPD_1IN54C_WIDTH / 8 ): (EPD_1IN54C_WIDTH / 8 + 1)) * EPD_1IN54C_HEIGHT;
//uint8_t DataBuffer[100];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_IWDG_Init(void);
static void MX_TIM7_Init(void);
void TaskUARTDecoder(void *argument);
void TaskScreenRefresh(void *argument);
void TaskCalculation(void *argument);
void TaskBlinkLED(void *argument);
void UARTTx(void *argument);
void TaskProcessor(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_SPI1_Init();
  MX_USART3_UART_Init();
  MX_IWDG_Init();
  MX_TIM7_Init();
  /* USER CODE BEGIN 2 */
	HAL_UART_Receive_DMA(&huart1,&UpperRx,1);// 启动DMA接收
	//DEV_Module_Init();//CS=0，DC=0，RST=1；
	HAL_TIM_Base_Start_IT(&htim7);
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of CalcSemaphore */
  CalcSemaphoreHandle = osSemaphoreNew(1, 1, &CalcSemaphore_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
	xSemaphoreTake(CalcSemaphoreHandle,0); //刷掉初始信号量
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of QueueUARTToAllocator */
  QueueUARTToAllocatorHandle = osMessageQueueNew (100, sizeof(uint8_t), &QueueUARTToAllocator_attributes);

  /* creation of QueueToUART */
  QueueToUARTHandle = osMessageQueueNew (16, sizeof(unsigned char *), &QueueToUART_attributes);

  /* creation of QueueDecoderToProcessor */
  QueueDecoderToProcessorHandle = osMessageQueueNew (16, sizeof(unsigned char *), &QueueDecoderToProcessor_attributes);

  /* creation of QueueProcessorToScreen */
  QueueProcessorToScreenHandle = osMessageQueueNew (2, sizeof(struct Image), &QueueProcessorToScreen_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of UARTDecoder */
  UARTDecoderHandle = osThreadNew(TaskUARTDecoder, NULL, &UARTDecoder_attributes);

  /* creation of ScreenRefresh */
  ScreenRefreshHandle = osThreadNew(TaskScreenRefresh, NULL, &ScreenRefresh_attributes);

  /* creation of Calculation */
  CalculationHandle = osThreadNew(TaskCalculation, NULL, &Calculation_attributes);

  /* creation of BlinkLED */
  BlinkLEDHandle = osThreadNew(TaskBlinkLED, NULL, &BlinkLED_attributes);

  /* creation of uarttx */
  uarttxHandle = osThreadNew(UARTTx, NULL, &uarttx_attributes);

  /* creation of processor */
  processorHandle = osThreadNew(TaskProcessor, NULL, &processor_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
	printf("Welcome!Potato Server Has Started!\r\n");
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

		
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_8;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_256;
  hiwdg.Init.Window = 4095;
  hiwdg.Init.Reload = 500;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 16;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 65535;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, DC_Pin|SPI_CS_Pin|RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : DC_Pin SPI_CS_Pin RST_Pin */
  GPIO_InitStruct.Pin = DC_Pin|SPI_CS_Pin|RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : BUSY_Pin */
  GPIO_InitStruct.Pin = BUSY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BUSY_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)//串口回调函数
{
	//printf(" !$");
	 if(huart->Instance==USART1)//上位机信号
	{
		BaseType_t xHigherPriorityTaskWoken=pdFALSE;
		if(xQueueSendFromISR(QueueUARTToAllocatorHandle , &UpperRx, &xHigherPriorityTaskWoken )!=pdTRUE)printf("**@$$");
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		


	}
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_TaskUARTDecoder */
/**
  * @brief  Function implementing the UARTDecoder thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_TaskUARTDecoder */
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
			//printf("：0x%x\r\n",Rx_queue);
		}
		else {
			printf("timeout!0x%lx LENGTH=%d\r\n",xReturn,i);
			if(Status == READDATA || Status == SUMCHECK) {vPortFree(data);}
			Status=WAITHEAD1;//回到初始状态
		}
		switch(Status)//接收状态机
		{
			case WAITHEAD1: if(Rx_queue==HEAD1){Status=WAITHEAD2;} else Status=WAITHEAD1; break;
			case WAITHEAD2: if(Rx_queue==HEAD2)Status=WAITHEAD3; else Status=WAITHEAD1; break;
			case WAITHEAD3: if(Rx_queue==HEAD3){CheckLenSum = 0; Status=GETLEN1; /*	printf("catch a frame!\r\n");*/}else Status=WAITHEAD1; break;
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
					printf("checklen wrong!\r\n");
					Status=WAITHEAD1;
				}
				break;
				//printf("length=%d\r\n",DataLen);
				//if(Command==TASK_SCREEN){Status=READIMAGE;/*printf("Taskscreen\r\n");*/}
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
				//printf("CheckSum=%d",CheckSum);
				if(Rx_queue==CheckSum){	
					Debug("recieve a frame\r\n");
					xQueueSend(QueueDecoderToProcessorHandle, &data, 10);
					data = NULL;
					//printf("receive a complete frame\r\n");
					//分配接收到的数据给子任务
					/*switch(Command)
					{
						case TASK_SCREEN:
							xReturn = xSemaphoreGive(ScreenSemaphoreHandle );//给出二值信号量
							if ( xReturn == pdTRUE ) 
								printf("TaskScreen:\r\n");
							else
							printf("TaskScreen Block!\r\n");
							break;
							
						case TASK_CALCULATION:
							xReturn = xSemaphoreGive(CalcSemaphoreHandle );//给出二值信号量
							if ( xReturn == pdTRUE ) {
								char* s = pvPortMalloc(sizeof(char)*100);
								sprintf(s, "TaskCalc:\r\n");
								xQueueSend(QueueToUARTHandle , &s, 100);
							}
							else
							printf("TaskCalc Block!\r\n");
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
							printf("taskscreen Fail!\r\n");
							break;
						case TASK_CALCULATION:
							printf("taskCalc Fail!\r\n");
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

/* USER CODE BEGIN Header_TaskScreenRefresh */
/**
* @brief Function implementing the ScreenRefresh thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_TaskScreenRefresh */
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
		//printf("disp------------------------\r\n");
		EPD_1IN54C_Display(image.BlackImage, image.YellowImage);//刷新屏幕
		//printf("close 5V, Module enters 0 power consumption ...\r\n");
		Debug("Screen Refresh Finish!\r\n");
    DEV_Module_Exit();
		vPortFree(image.BlackImage);
		vPortFree(image.YellowImage);
  }
  /* USER CODE END TaskScreenRefresh */
}

/* USER CODE BEGIN Header_TaskCalculation */
/**
* @brief Function implementing the Calculation thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_TaskCalculation */
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
			/*printf("task calc!\r\n");
			compiled_size = parse_exp((char*)DataBuffer, compiled_code, sizeof(compiled_code), &error_pos);
			if (!compiled_size)printf("Parse error at position %u\n", (unsigned)error_pos);
			val = eval_compiled(compiled_code, compiled_size, variable_values);
			printf("result = %d\n", val);*///轻量化实现
			//13K内存实现:
			sysclock1 =(short)(TIM7 -> CNT);
			//e= expr_create((char*)DataBuffer, strlen((char*)DataBuffer), &vars, NULL);
			if (e == NULL) {
				printf("Syntax error!\r\n");
				expr_destroy(e,&vars);
			}
			else{
				result = expr_eval(e);
				expr_destroy(e,&vars);
				sysclock2 =(short)(TIM7 -> CNT);
				printf("result= %.5f  time=%uus\r\n", result,(short)(sysclock2-sysclock1));
			}

		}

  }
  /* USER CODE END TaskCalculation */
}

/* USER CODE BEGIN Header_TaskBlinkLED */
/**
* @brief Function implementing the BlinkLED thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_TaskBlinkLED */
void TaskBlinkLED(void *argument)
{
  /* USER CODE BEGIN TaskBlinkLED */
  /* Infinite loop */
  for(;;)
  {	
		HAL_IWDG_Refresh(&hiwdg);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
		osDelay(1);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
		osDelay(1000);
		 
  }
  /* USER CODE END TaskBlinkLED */
}

/* USER CODE BEGIN Header_UARTTx */
/**
* @brief Function implementing the uarttx thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_UARTTx */
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
		if (pdTRUE == xReturn){
			printf("%s",s);
			vPortFree(s);
		}
  }
  /* USER CODE END UARTTx */
}

/* USER CODE BEGIN Header_TaskProcessor */
/**
* @brief Function implementing the processor thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_TaskProcessor */
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
						printf("Syntax error!\r\n");
						expr_destroy(e,&vars);
					}
					else{
						result = expr_eval(e);
						expr_destroy(e,&vars);
						sysclock2 =(short)(TIM7 -> CNT);
						printf("result= %.5f  time=%uus\r\n", result,(short)(sysclock2-sysclock1));
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

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM16 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM16) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

	
  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
