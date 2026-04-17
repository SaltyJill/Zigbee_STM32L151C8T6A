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
#define heart_beat_len 4
#define cmd_len 4
#define tx_len 24
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t cmd_buff[cmd_len];
uint8_t tx_pack[tx_len] = {
    netID | netPT, pointADD, // 本节点地址
    0xFB, 0xFB, 0xFC, 0xFC,  // 数据帧头
    0x00,                    // 6数据类型，Default
    0x00, 0x00,               // 7-8电池电量，Default
    0x00,                    // 9信号类型，Default
    0x00, 0x00, 0x00, 0x00,  // 10-13累计发送次数，Default
    0x00, 0x00, 0x00, 0x00,  // 14-17数据，Default
    0x00, 0x00,              // 18-19CRC校验，Default
    0xFE, 0xFE, 0xFF, 0xFF   // 数据帧尾
};
uint16_t u16CRCtemp = 0;
volatile uint8_t Flag_rx = RESET;
volatile uint8_t Flag_tx = RESET;
volatile uint8_t Run_state = RESET; // RESET-Lowpower,SET-RUN
uint8_t heart_beat[heart_beat_len] = {netID | netPT, pointADD, 0x00, 0xC0};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void LED_flash200(uint8_t count)
{
    for (uint8_t i = 0; i < count; i++)
    {
        led_on();
        HAL_Delay(100);
        led_off();
        HAL_Delay(100);
    }
}
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
    MX_USART3_UART_Init();
    MX_RTC_Init();
    /* USER CODE BEGIN 2 */
    pvt_low_Power_init();
    pvt_Zigbee_init();
    HAL_Delay(1000); // 用于烧录程序延时并等待Zigbee入网(后续可调试为zigbee入网时间)
    HAL_UART_Receive_DMA(&huart3, cmd_buff, cmd_len);
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        led_on();
        /*---MISSION HERE---*/
        
        // CRC校验
        u16CRCtemp = CRC16_IBM_Byte(tx_pack, 7, 18);
        tx_pack[18] = (uint8_t)(u16CRCtemp >> 8);
        tx_pack[19] = (uint8_t)(u16CRCtemp & 0x00FF);
        HAL_UART_Transmit_DMA(&huart3, tx_pack, tx_len);
        /*---COMMAND PROCESSPN---*/
        if (Flag_rx == SET)
        {
            if (cmd_buff[3] == 0xC0)
            {
                if (cmd_buff[2] == 'A')
                {
                    Run_state = SET;
                }
                else if (cmd_buff[2] == 'B')
                {
                    Run_state = RESET;
                }
            }
            HAL_UART_Receive_DMA(&huart3, cmd_buff, cmd_len);
            Flag_rx = RESET;
        }
        LED_flash200(3);
        if (Run_state == RESET)
        {
            pvt_low_Power_enterSTOP();
            /*---STOP HERE---*/
            pvt_low_Power_exitSTOP();
            heart_beat[2] = Run_state;
            HAL_UART_Transmit_DMA(&huart3, heart_beat, heart_beat_len);
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
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSE;
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
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
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
void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{
    ;
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)
    {
        Flag_rx = SET;
    }
}
void HAL_uart_txCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)
    {
        Flag_tx = SET;
    }
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
