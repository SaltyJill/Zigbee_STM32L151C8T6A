/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "adc.h"
#include "dma.h"
#include "rtc.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stop.h"
#include "zigbee.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define len 4
#define len_data 24
#define noDATA 0x10
#define sendDATA 0x11
#define lowPower 0x10
#define normal 0x11
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
//volatile uint16_t ADC_cnt=0;
//volatile uint16_t ADC_val=0;
uint8_t work_mode=normal;//0x10-lowpower,0x11-normal,swith by order
uint8_t work_data=sendDATA;
uint16_t Vbat=0;
uint32_t CNTsend=0;
//uint8_t buf_packLoader[4];
uint8_t buf_CMD[len];
uint16_t CRC_res=0;
volatile uint8_t flag_CMD=RESET;
//0x24-帧头帧尾，work_data:0x10不需要数据包0x11需要，work_mode:0x10低功耗模式0x11正常工作
uint8_t heart_beat[len]={0x24,0x10,0x10,0x24};
uint8_t Data_pack[len_data]={
	netID | netPoint, pointADD,//节点地址
	0xFB,0xFB,0xFC,0xFC,//数据帧头
	0x01,
	0x00,0x00,//电池电量，电池电压mV
	0x01,
	0x00,0x00,0x00,0x00,//发送次数
	0x01,0x02,0x03,0x04,
	0x00,0x00,//CRC校验
	0xFE,0xFE,0xFF,0xFF//数据帧尾
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void u16MOVu8(uint32_t data, uint8_t *pack, uint8_t pos);
void u32MOVu8(uint32_t data, uint8_t *pack, uint8_t pos);
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
  MX_RTC_Init();
  MX_USART3_UART_Init();
  MX_ADC_Init();
  /* USER CODE BEGIN 2 */
	Lp_Init();
	HAL_Delay(1000);//等待电平稳定
	Zigbee_CFGinit(Termminal);
	HAL_Delay(1000);//烧录,联网延时
	HAL_UART_Transmit_DMA(&huart3,(uint8_t *)"Link Start\n",11);
	HAL_UART_Receive_DMA(&huart3,buf_CMD,len);//等待中心节点指令
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		led_on;
		HAL_Delay(100);
		//HAL_Delay(1000);
		//---处理中心节点指令---
		if(flag_CMD==SET)
		{
			//---CMD JUDGE HERE---
			if(buf_CMD[0]==buf_CMD[3]&&buf_CMD[0]==0x24)
			{
				work_data=buf_CMD[1];
				work_mode=buf_CMD[2];
			}
			flag_CMD=RESET;
		}
		//---数据包数据处理和装填---
		if(work_data==sendDATA)
		{
			Vbat=Battery_mV_Avg(8);//ADC获取电池电压
			CNTsend++;
			u32MOVu8(CNTsend,Data_pack,11);
			u16MOVu8(Vbat,Data_pack,8);
			CRC_res=CRC16_IBM_Byte(Data_pack,7,18);
			u16MOVu8(CRC_res,Data_pack,19);
			HAL_UART_Transmit_DMA(&huart3,Data_pack,len_data);//发送数据包
			// 在 Lp_STOPenter() 之前添加等待逻辑：
      while (huart3.gState != HAL_UART_STATE_READY) {}//等待数据包发送
		}
		else if(work_data==noDATA)
		{
			work_mode=lowPower;//无数据处理任务，自动启用低功耗模式
			HAL_Delay(1500);//低功耗无任务等待命令
		}
		//---低功耗模式进入和正常模式发送间隔---
		if(work_mode==lowPower)
		{
			Lp_STOPenter();
			//---STOP HERE---
			Lp_STOPexit();
			HAL_Delay(500);//苏醒后联网
			heart_beat[1]=work_data;
			heart_beat[2]=work_mode;
			HAL_UART_Transmit_DMA(&huart3,heart_beat,len);//发送心跳包
			while (huart3.gState != HAL_UART_STATE_READY) {}
			HAL_UART_Receive_DMA(&huart3,buf_CMD,len);//接受命令
		}
		else if(work_mode==normal)
		{
			for(uint8_t i=0;i<5;i++)
			{
				led_on;
				HAL_Delay(500);
				led_off;
				HAL_Delay(500);
			}
		}
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance==USART3)
	{
		HAL_UART_Receive_DMA(&huart3,buf_CMD,len);//再次接受中心节点指令
		flag_CMD=SET;
	}
}

void u32MOVu8(uint32_t data, uint8_t *pack, uint8_t pos)
{
    pos--;
    for (uint8_t i = 0, j = 3; i < 4; i++, j--)
    {
        pack[pos + i] = (uint8_t)(data >> 8 * j);
    }
}
void u16MOVu8(uint32_t data, uint8_t *pack, uint8_t pos)
{
    pos--;
    pack[pos] = (uint8_t)(data >> 8);
    pack[pos + 1] = (uint8_t)(data & 0x00FF);
}
/* USER CODE END 4 */

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
#ifdef USE_FULL_ASSERT
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
