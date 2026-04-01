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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define RX_LEN 25
#define RX_COMMEND_LEN 4
#define TX_LEN 15
uint8_t RX_BUF[RX_LEN];
uint8_t RX_COMEND[RX_COMMEND_LEN];
uint8_t TX_BUF[TX_LEN];
volatile uint8_t Flag_RX_uart1 = RESET;
volatile uint8_t Flag_RX_uart3 = RESET;
volatile uint8_t Flag_CMD_ILLEGAL = RESET;
/*---ZIGBEE CFG---*/
uint8_t Zigbee_CFGget[2] = {0x23, 0xA0}; // 查询CFG,返回0xA2+14byte
uint8_t Zigbee_CFGset[2] = {0x23, 0xFE}; // 设置CFG
uint8_t Zigbee_CFGrst[2] = {0x23, 0x23}; // 模块重启,退出CFG 必须高电平
uint8_t Zigbee_CFG[14] = {
    0x00,
    0x00, // 节点地址 中心节点固定0000,有效范围0001-FFFE
    0xA0, // 网络ID   00-FF
    0x01, // 网络类型 01-网状网,02-星型网,07-对等网
    0x01, // 节点类型 01-中心节点,03-中继路由,04-终端节点
    0x02, // 发送模式 01-广播,02-主从,03-点对点
    0x06, // 波特率   01-1200,02-2400,03-4800,04-9600,05-19200,06-38400,07-57600,08-115200
    0x01, // 奇偶校验 01-NONE，02-EVEN，03-ODD
    0x01, // 数据格式 01-8位
    0x02, // 地址编码 01-ACSII，02-HEX
    0xFF, // 串口超时 05-FF
    0x08, // 无线频点 00-0F
    0x00, // 发射功率 缺省
    0x01  // 源地址   01-不输出,02-ASCII 输出,03-HEX输出
};
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void Zigbee_init(void)
{
    set_enter;
    HAL_Delay(3000);
    HAL_UART_Transmit(&huart3, Zigbee_CFGset, 2, 100);
    HAL_UART_Transmit(&huart3, Zigbee_CFG, 14, 500);
    set_exit;
    HAL_UART_Transmit(&huart3, Zigbee_CFGrst, 2, 100);
}
void TX_CMD_Load(uint8_t *padd_Buf, uint8_t status)
{
    TX_BUF[0] = padd_Buf[2] - '0';
    TX_BUF[1] = padd_Buf[3] - '0';
    if (status == SET)
    {
        TX_BUF[2] = 0xC0;
        TX_BUF[3] = 0x0B;
    }
    else
    {
        TX_BUF[2] = 0xC0;
        TX_BUF[3] = 0x0A;
    }
}
void Clear_Buf(uint8_t *padd_Buf, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++)
    {
        padd_Buf[i] = 0;
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
    MX_USART1_UART_Init();
    MX_USART3_UART_Init();
    /* USER CODE BEGIN 2 */
    HAL_UART_Transmit_DMA(&huart1, (uint8_t *)"Wait for Zigbee init...\r\n", 27);
    Zigbee_init();
    HAL_UART_Transmit_DMA(&huart1, (uint8_t *)"System is ready.\r\n", 17);
    HAL_UART_Receive_DMA(&huart3, RX_BUF, RX_LEN);
    HAL_UART_Receive_DMA(&huart1, RX_COMEND, RX_COMMEND_LEN);
    __HAL_DMA_DISABLE_IT(&hdma_usart3_rx, DMA_IT_HT);
    __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /*---ZIGBEE RECEIVING---*/
        if (Flag_RX_uart3 == SET)
        {
            
            Flag_RX_uart3 = RESET;
        }
        /*---COMMAND PROCESSING---*/
        if (Flag_RX_uart1 == SET)
        {
            if (RX_COMEND[0] == '#')
            {
                led_on;
                switch (RX_COMEND[1])
                {
                case 'S':
                    TX_CMD_Load(RX_COMEND, SET);
                    break;
                case 'W':
                    TX_CMD_Load(RX_COMEND, RESET);
                    break;
                default:
                    HAL_UART_Transmit_DMA(&huart1, (uint8_t *)"Invalid Command\r\n", 17);
                    Flag_CMD_ILLEGAL = SET;
                    break;
                }
                if (Flag_CMD_ILLEGAL == RESET)
                {
                    HAL_UART_Transmit_DMA(&huart3, TX_BUF, 4);
                }
                else
                {
                    Flag_CMD_ILLEGAL = RESET;
                }
            }
            else
            {
                HAL_UART_Transmit_DMA(&huart1, (uint8_t *)"Invalid Command\r\n", 17);
            }
            Flag_RX_uart1 = RESET;
            Clear_Buf(TX_BUF, 4);
            HAL_UART_Receive_DMA(&huart1, RX_COMEND, RX_COMMEND_LEN); // 继续接收命令
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

    /** Configure the main internal regulator output voltage
     */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL4;
    RCC_OscInitStruct.PLL.PLLDIV = RCC_PLL_DIV2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        Flag_RX_uart1 = SET;
    }
    else if (huart->Instance == USART3)
    {
        Flag_RX_uart3 = SET;
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
