#include "stop.h"
uint32_t rtc_slpTime = 9;
extern void SystemClock_Config(void);
// 程序低功耗的准备工作
void pvt_low_Power_init(void)
{
    __HAL_RCC_COMP_CLK_DISABLE();    // 禁用比较器
    HAL_PWR_DisablePVD();            // 禁用PVD电源检测
    HAL_PWREx_EnableUltraLowPower(); // 关闭VREFINT参考电压
    HAL_PWREx_EnableFastWakeUp();    // 忽略VREFINT快速启动
    HAL_DBGMCU_DisableDBGStopMode(); // STOP模式禁用串口调试
    HAL_PWR_EnableBkUpAccess();      // 启用备份STOP模式下RTC访问
    // pvt_RTCinit();                          // 配置RTC
    HAL_RTCEx_DeactivateWakeUpTimer(&hrtc); // 禁止RTC内部唤醒计数
    // HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, DEVICE_RTC_INTERNAL_TIME, RTC_WAKEUPCLOCK_CK_SPRE_16BITS);//启用RTC内部唤醒 10s定时
    // HAL_Delay(1000); // 用于烧录程序延时
}
// 进入STOP模式前的准备工作
void pvt_low_Power_enterSTOP(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    led_off();
    /*---ZIGBEE休眠--- */
    slp_enter();
    HAL_Delay(50);
    /*---STOP串口设置*/
    HAL_UART_DeInit(&huart3);                               // 进入STOP模式前关闭串口
    __HAL_UART_CLEAR_FLAG(&huart3, UART_FLAG_RXNE);         // 清除串口接收标志，防止进入STOP模式后被误唤醒
    __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&hrtc, RTC_FLAG_WUTF); // 清除RTC唤醒标志，防止进入STOP模式后被误唤醒
    // 串口模拟输入
    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    /*---设置RTC唤醒时间并进入STOP模式---*/
    HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, rtc_slpTime, RTC_WAKEUPCLOCK_CK_SPRE_16BITS);
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
}
// 退出STOP模式后的恢复工作
void pvt_low_Power_exitSTOP(void)
{
    slp_exit();                             // Zigbee唤醒
    SystemClock_Config();                   // 恢复系统时钟
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);      // 清除唤醒标志
    MX_GPIO_Init();                         // 恢复GPIO
    MX_USART3_UART_Init();                  // 恢复串口
    HAL_RTCEx_DeactivateWakeUpTimer(&hrtc); // 禁止RTC内部唤醒计数
}
