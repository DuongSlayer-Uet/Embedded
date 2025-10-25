/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BMP180_ADDR (0x77 << 1)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;

osThreadId defaultTaskHandle;
/* USER CODE BEGIN PV */
osThreadId SenDataTaskHandle;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
void StartDefaultTask(void const * argument);

/* USER CODE BEGIN PFP */
void SendDataTask(void const * argument);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

osSemaphoreId sensorSemHandle;
osSemaphoreDef(sensorSem);

osSemaphoreId TxSemHandle;
osSemaphoreDef(TxSem);

// Global variables

typedef struct {
    uint16_t temperature;  // slave_data[0]
    uint16_t pressure;     // slave_data[1]
    uint16_t humidity;     // slave_data[2]
} SlaveData_t;

volatile SlaveData_t slave_data;

// Calib variables
int16_t AC1, AC2, AC3, B1, B2, MB, MC, MD;
uint16_t AC4, AC5, AC6;

// UART1
uint8_t rx_buff[100];
uint32_t rx_index = 0;
uint8_t rx_byte;

// Modbus
uint16_t ModbusIntervalData = 1000;


// Đọc 16-bit từ BMP180
int16_t BMP180_Read16(uint8_t reg)
{
    uint8_t buf[2];
    HAL_I2C_Mem_Read(&hi2c1, BMP180_ADDR, reg, 1, buf, 2, HAL_MAX_DELAY);
    return (buf[0] << 8) | buf[1];
}

// Read Calib data
void BMP180_ReadCalibration(void)
{
    AC1 = BMP180_Read16(0xAA);
    AC2 = BMP180_Read16(0xAC);
    AC3 = BMP180_Read16(0xAE);
    AC4 = BMP180_Read16(0xB0);
    AC5 = BMP180_Read16(0xB2);
    AC6 = BMP180_Read16(0xB4);
    B1  = BMP180_Read16(0xB6);
    B2  = BMP180_Read16(0xB8);
    MB  = BMP180_Read16(0xBA);
    MC  = BMP180_Read16(0xBC);
    MD  = BMP180_Read16(0xBE);
}

// Đọc nhiệt độ (T/10 = độ)
uint16_t BMP180_ReadTemperature(void)
{
    uint8_t cmd = 0x2E;
    uint8_t buf[2];

    // Trigger đo nhiệt độ
    HAL_I2C_Mem_Write(&hi2c1, BMP180_ADDR, 0xF4, 1, &cmd, 1, 1000);
    osDelay(5); // chờ ít nhất 4.5 ms

    // Đọc raw temperature
    HAL_I2C_Mem_Read(&hi2c1, BMP180_ADDR, 0xF6, 1, buf, 2, 1000);
    int32_t UT = (buf[0] << 8) | buf[1];

    // Quy đổi
    int32_t X1 = ((UT - (int32_t)AC6) * (int32_t)AC5) >> 15;
    int32_t X2 = ((int32_t)MC << 11) / (X1 + MD);
    int32_t B5 = X1 + X2;
    int16_t T = (B5 + 8) >> 4;
    return T;
}

// Hàm tính CRC16 Modbus (đa thức 0xA001)
void Modbus_CRC16(uint8_t *data, uint16_t length, uint8_t *crc_low, uint8_t *crc_high)
{
    uint16_t crc = 0xFFFF;

    for (uint16_t pos = 0; pos < length; pos++)
    {
        crc ^= data[pos]; // XOR byte

        for (uint8_t i = 0; i < 8; i++)
        {
            if (crc & 0x0001)   // nếu bit LSB = 1
            {
                crc >>= 1;
                crc ^= 0xA001;  // đa thức Modbus
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    // Trả kết quả: CRC low trước, high sau (theo chuẩn Modbus RTU)
    *crc_low  = crc & 0xFF;
    *crc_high = (crc >> 8) & 0xFF;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	uint8_t crc_l;
	uint8_t crc_h;

	if (huart->Instance == USART1)
	{
        rx_buff[rx_index++] = rx_byte;  // lưu byte nhận được

        // Nhận đủ 1 frame (8byte)
        if(rx_index == 6)
        {
        	// Check CRC
        	Modbus_CRC16(rx_buff, 4, &crc_l, &crc_h);
        	// Nếu CRC match
        	if((crc_l == rx_buff[4]) && (crc_h == rx_buff[5]))
        	{
        		// Check xem gói tin này có phải dành cho mình
        		if(rx_buff[0] == 0x05)
        		{
        			// Đúng, tách frame lấy data
        			// Nếu Func code == 0x04 => read data
        			if(rx_buff[1] == 0x04)
        			{
						// Nhả block task read temp sensor
						osSemaphoreRelease(TxSemHandle);
        			}
        			// Có thể add thêm các function khác
        		}
        	}
        	rx_index = 0;
        }
        // Tiếp tục nhận byte tiếp theo
        HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}

void FREERTOS_Init()
{
	  sensorSemHandle = osSemaphoreCreate(osSemaphore(sensorSem), 1);
	  osSemaphoreWait(sensorSemHandle, 0); // reset về 0
	  TxSemHandle = osSemaphoreCreate(osSemaphore(TxSem), 1);
	  osSemaphoreWait(TxSemHandle, 0); // reset về 0
}

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
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  FREERTOS_Init();
  BMP180_ReadCalibration();
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  osThreadDef(SendTask, SendDataTask, osPriorityNormal, 0, 128);
  SenDataTaskHandle = osThreadCreate(osThread(SendTask), NULL);
  /* USER CODE END RTOS_THREADS */

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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
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

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
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
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

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
  huart1.Init.BaudRate = 9600;
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
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_12|GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB12 PB13 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_12|GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void SendDataTask(void const * argument)
{
	uint8_t tx_buff[6];
	uint8_t crc_l;
	uint8_t crc_h;
	for(;;)
	{
	    // wait, chờ đến khi có request
	    if (osSemaphoreWait(TxSemHandle, osWaitForever) == osOK)
	    {
			// Nếu addr data = 0x00 => master muốn đọc temp
			if(rx_buff[3] == 0)
			{
				tx_buff[0] = 0x05;
				tx_buff[1] = rx_buff[1];
				tx_buff[2] = (uint8_t)(slave_data.temperature >> 8);   // high byte
				tx_buff[3] = (uint8_t)(slave_data.temperature & 0xFF); // low byte
	        	// Check CRC
	        	Modbus_CRC16(tx_buff, 4, &crc_l, &crc_h);
	        	tx_buff[4] = crc_l;
	        	tx_buff[5] = crc_h;
	        	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
	        	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
	        	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
				HAL_UART_Transmit_IT(&huart1, tx_buff, 6);

			}
	    }
	}
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN 5 */
	HAL_UART_Receive_IT(&huart1, &rx_byte, 1); // nhận 1 byte
  /* Infinite loop */
  for(;;)
  {
	   slave_data.temperature = BMP180_ReadTemperature();
	  //slave_data.temperature = 10;
	   osDelay(100);
  }
  /* USER CODE END 5 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
