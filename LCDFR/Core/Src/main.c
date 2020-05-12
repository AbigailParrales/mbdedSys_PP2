/************************************************************************
* Copyright 2020 ITESM                                                  *
*                                                                       *
*                                                                       *
* Second Partial Project                                                *
*                                                                       *
* Authors:                                                              *
* Jesús Enrique Luna Medina          A01632334                          *
* Daniela abigail Parrales Mejía     A01228629                          *
* Luis Cortés Leal                   A01631163                          *
*                                                                       *
* May  2020                                                             *
*                                                                       *
* This is a temperature sensor that reports in serial port (UART)		*
* and in a 16x2 LCD I2C													*
*                                                                       *
************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "i2c-lcd.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;

osThreadId TaskTempHandle;
osThreadId Task_ToggleHandle;
osMutexId uartMutexHandle;
osThreadId Task_LCDHandle;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
void TaskTemp_init(void const * argument);
void Task_Toggle_init(void const * argument);
void Task_LCD_init(void const * argument);
void uart_Temp(void);

uint32_t value[1];
uint8_t data[]="Temp:    ,LEDMAX:    ,LEDMIN:    \r\n";


int main(void)
{

  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();

  lcd_init();

  HAL_ADC_Start_DMA(&hadc1, value, 1);

  /* Create the mutex(es) */
  /* definition and creation of uartMutex */
  osMutexDef(uartMutex);
  uartMutexHandle = osMutexCreate(osMutex(uartMutex));

  /* Create the thread(s) */
  /* definition and creation of TaskTemp */
  osThreadDef(TaskTemp, TaskTemp_init, osPriorityNormal, 0, 128);
  TaskTempHandle = osThreadCreate(osThread(TaskTemp), NULL);

  /* definition and creation of Task_Toggle */
  osThreadDef(Task_Toggle, Task_Toggle_init, osPriorityNormal, 0, 128);
  Task_ToggleHandle = osThreadCreate(osThread(Task_Toggle), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  osThreadDef(Task_LCD, Task_LCD_init, osPriorityNormal, 0, 128);
  Task_LCDHandle = osThreadCreate(osThread(Task_LCD), NULL);
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();
 
  while (1){

  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE END ADC1_Init 1 */
  /** Common config 
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel 
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

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
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12|GPIO_PIN_13, GPIO_PIN_SET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PB12 PB13 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/**
  * @brief  Function implementing the TaskTemp thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_TaskTemp_init */
void TaskTemp_init(void const * argument)
{
  for(;;)
  {
	HAL_ADC_Start_DMA(&hadc1, value, 1);
	osDelay(500);

	xSemaphoreTake(uartMutexHandle, portMAX_DELAY);
	HAL_UART_Transmit(&huart1, data, 35 ,1000);
	xSemaphoreGive(uartMutexHandle);
    osDelay(500);//cada seg
  }
}

/* USER CODE BEGIN Header_Task_Toggle_init */
/**
* @brief Function implementing the Task_Toggle thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Task_Toggle_init */
void Task_Toggle_init(void const * argument)
{
  for(;;)
  {
	 xSemaphoreTake(uartMutexHandle, portMAX_DELAY);
	 HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
	 xSemaphoreGive(uartMutexHandle);
    osDelay(500); /* !< Every second */
  }
}

void Task_LCD_init(void const * argument)
{
  for(;;)
  {
	lcd_send_cmd(0x80);
	  lcd_send_string("TEMP:");
	  lcd_send_data((value[0]/1000)+48);          /* !< decimal */
	  lcd_send_data(((value[0]%1000)/100)+48);    /* !< unit */
	  lcd_send_string(".");
	  lcd_send_data(((value[0]%100)/10)+48);      /* !< decimal point */
	  lcd_send_string(" C     ");
	  lcd_send_cmd(0xc0);                         /* !< second line */
	  lcd_send_string("                ");

	  HAL_Delay(200);

	  /* !< LED conditional indicators*/
	  	  if(((value[0]/1000)+48)>50){
			  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);    /* !< ON */
			  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);      /* !< OFF */
			  data[18]='O', data[19]='n', data[20]=' ';
			  data[30]='O', data[31]='f', data[32]='f';
	  	  }

	  	  if(((value[0]/1000)+48)<50){
	  		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);    /* !< OFF */
			  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);    /* !< ON */
			  data[18]='O', data[19]='f', data[20]='f';
			  data[30]='O', data[31]='n', data[32]=' ';
	  	  }
	  	  if(((value[0]/1000)+48) == 50){
	  		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);    /* !< OFF */
	  		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);    /* !< ON */
	  		  data[18]='O', data[19]='f', data[20]='f';
	  		  data[30]='O', data[31]='f', data[32]='f';
	  	  }


	  	/* !< Print temp in uart */
	  	uart_Temp();

    osDelay(1000); /* !< Every second */
  }
}

 /**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void uart_Temp(void){
	  data[6]=((value[0]/1000)+48);
	  data[7]=(((value[0]%1000)/100)+48);
	  data[8]=('C');
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
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