#include "stop.h"

static uint32_t SLPtime = 9;
extern void SystemClock_Config(void);
//extern void MX_GPIO_Init(void);

// 其他低功耗配置
void Lp_Init(void)
{
    __HAL_RCC_COMP_CLK_DISABLE();    // 禁用比较器
    HAL_PWR_DisablePVD();            // 禁用PVD电源检测
    HAL_PWREx_EnableUltraLowPower(); // 关闭VREFINT参考电压
    HAL_PWREx_EnableFastWakeUp();    // 忽略VREFINT快速启动
    HAL_DBGMCU_DisableDBGStopMode(); // 禁用串口调试
    HAL_PWR_EnableBkUpAccess();      // 启用STOP模式下RTC访问
    // HAL_PWR_DisableBkUpAccess();
		pvt_RTCinit();
    HAL_RTCEx_DeactivateWakeUpTimer(&hrtc); // 禁用RTC计数
    // HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, DEVICE_RTC_INTERNAL_TIME, RTC_WAKEUPCLOCK_CK_SPRE_16BITS);
}

void pvt_RTCinit(void)
{
	  hrtc.Instance = RTC;
    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv = 127;
    hrtc.Init.SynchPrediv = 255;
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
    HAL_RTC_Init(&hrtc);
}

void Lp_STOPenter(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    //---外设STOP---
    Zigbee_SLPenter;
    led_off;
    // HAL_Delay(10);
    HAL_UART_DeInit(&huart3);                               // 释放串口资源
    __HAL_UART_CLEAR_FLAG(&huart3, UART_FLAG_RXNE);         // 清除串口接收标志，防止进入STOP模式后被误唤醒
    __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&hrtc, RTC_FLAG_WUTF); // 清除RTC唤醒标志，防止进入STOP模式后被误唤醒
    //---串口模拟输入---
    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    //---停止GPIO时钟---
    __HAL_RCC_GPIOB_CLK_DISABLE(); // 禁用GPIO时钟
    __HAL_RCC_ADC1_CLK_DISABLE();  // 禁用ADC时钟
    //---设置休眠时间并进入休眠---
    HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, SLPtime, RTC_WAKEUPCLOCK_CK_SPRE_16BITS);
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
}

void Lp_STOPexit(void)
{
    SystemClock_Config();              // STOP模式唤醒之后，默认运行MSI时钟，如需要HSI或HSE时钟，需要重新配置系统时钟
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU); // 清楚唤醒标记
    MX_GPIO_Init();                    // GPIO初始化
    __HAL_RCC_GPIOB_CLK_ENABLE();      // 启用GPIO时钟
    __HAL_RCC_ADC1_CLK_ENABLE();       // 启用ADC时钟
    //HAL_UART_Init(&huart3);            // 串口初始化
    MX_USART3_UART_Init();
		HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);//禁用RTC计数
    // 启用外设
    Zigbee_SLPexit;
    led_on;
    // HAL_Delay(10);
}
