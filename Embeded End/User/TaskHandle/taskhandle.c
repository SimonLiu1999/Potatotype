#include "taskhandle.h"

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
		//printf("led\r\n");
		HAL_IWDG_Refresh(&hiwdg);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
		osDelay(1);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
		osDelay(1000);
		 
  }
  /* USER CODE END TaskBlinkLED */
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
