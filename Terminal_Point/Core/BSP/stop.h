#ifndef __STOP_H
#define __STOP_H

#include "main.h"
#include "rtc.h"
#include "usart.h"
#include "gpio.h"
// 程序低功耗的准备工作
void pvt_low_Power_init(void);
// 进入STOP模式前的准备工作
void pvt_low_Power_enterSTOP(void);
// 退出STOP模式后的恢复工作
void pvt_low_Power_exitSTOP(void);
#endif
