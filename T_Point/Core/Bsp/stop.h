#ifndef __STOP_H
#define __STOP_H

#include "main.h"
#include "rtc.h"
#include "usart.h"
#include "gpio.h"

void Lp_Init(void);
void Lp_STOPenter(void);
void Lp_STOPexit(void);
void pvt_RTCinit(void);

#endif
