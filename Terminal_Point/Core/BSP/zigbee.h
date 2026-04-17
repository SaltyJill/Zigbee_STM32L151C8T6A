#ifndef __ZIGBEE_H
#define __ZIGBEE_H

#include "main.h"
#include "usart.h"

#define netID 0xA0
#define netPT 0x08
#define pointADD 0x01
void pvt_Zigbee_init(void);
// CRC16-IBM校验函数，返回计算得到的CRC值
uint16_t CRC16_IBM_Byte(uint8_t *data,uint8_t start,uint8_t end);

#endif
