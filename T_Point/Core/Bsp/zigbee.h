#ifndef __ZIGBEE_H
#define __ZIGBEE_H

#include "main.h"

#define MainPoint 1
#define Terminal 0
//--- 网络设置 ---
// 终端节点地址构成，0x(netID|netPoint)+0x(pointADD)
#define netID 0x50    // 网络ID区分组
#define netPoint 0x0E // 频点，区分频道。推荐4-2.425,9-2.450,E-2.475,F-2.480,避免WiFi干扰
#define pointADD 0x50 // 节点地址
//参数可以是 MainPoint 或者 Termminal
void Zigbee_CFGinit(uint8_t CFGname);
//CRC校验
uint16_t CRC16_IBM_Byte(uint8_t *data,uint8_t start,uint8_t end);

#endif
