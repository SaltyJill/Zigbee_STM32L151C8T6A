#include "zigbee.h"
#include "main.h"

// ZIGBEE CFG
// uint8_t ZIGBEE_CFGget[2] = {0x23, 0xA0}; // 查询参数,返回A2+14byte
uint8_t ZIGBEE_CFGset[2] = {0x23, 0xFE}; // 设置参数
uint8_t ZIGBEE_CFGrst[2] = {0x23, 0x23}; // 设备重启 CFG 必须高电平
uint8_t TERMMINAL_CFG[14] = {
    netID | netPT, pointADD, // 节点地址 中心节点固定0000,有效范围0001-FFFE
    netID,                   // 网络ID   00-FF
    0x02,                    // 网络类型 01-网状网,02-星型网,07-对等网
    0x04,                    // 节点类型 01-中心节点,03-中继路由,04-终端节点
    0x02,                    // 发送模式 01-广播,02-主从,03-点对点
    0x06,                    // 波特率   01-1200,02-2400,03-4800,04-9600,05-19200,06-38400,07-57600,08-115200
    0x01,                    // 奇偶校验 01-NONE，02-EVEN，03-ODD
    0x01,                    // 数据格式 01-8位
    0x02,                    // 地址编码 01-ACSII，02-HEX
    0xFF,                    // 串口超时 05-FF
    netPT,                   // 无线频点 00-0F
    0x00,                    // 发射功率 缺省
    0x01                     // 源地址   01-不输出,02-ASCII 输出,03-HEX输出
};
void pvt_Zigbee_init(void)
{
    cfg_enter();
    HAL_Delay(3000);
    HAL_UART_Transmit(&huart3, ZIGBEE_CFGset, 2, 100);
    HAL_UART_Transmit(&huart3, TERMMINAL_CFG, 14, 500);
    cfg_exit();
    HAL_UART_Transmit(&huart3, ZIGBEE_CFGrst, 2, 100);
}
uint16_t CRC16_IBM_37(uint8_t *data)
{
    uint16_t crc = 0x0000;  // 初始值 (Init)
    uint16_t poly = 0xA001; // 反转后的多项式 (0x8005 的反转)

    for (uint16_t i = 2; i < 7; i++)
    {
        crc ^= data[i]; // 将字节数据与初始值低8位异或

        for (uint16_t j = 0; j < 8; j++)
        {
            if (crc & 0x0001)
            {
                // 如果最低位为 1，右移并与多项式异或
                crc = (crc >> 1) ^ poly;
            }
            else
            {
                // 如果最低位为 0，直接右移
                crc >>= 1;
            }
        }
    }

    return crc; // 结果异或值 (XorOut) 为 0x0000，直接返回
}