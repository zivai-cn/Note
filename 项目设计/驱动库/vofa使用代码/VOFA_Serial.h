#ifndef __VOFA_SERIAL_H
#define __VOFA_SERIAL_H

#include "stm32f10x.h"
#include <stdint.h>

/* 可根据目标项目修改以下配置 */
#define VOFA_USART           USART1
#define VOFA_BAUDRATE        9600U
#define VOFA_RX_BUFFER_SIZE  100U

void VOFA_Init(void);
void VOFA_SendString(const char *text);
void VOFA_Printf(const char *format, ...);

/* 在 RX 中断中调用此函数，逐字节传入接收到的 UART 数据 */
void VOFA_InputByte(uint8_t byte);

/* 收到以 '\n' 结尾的完整一行后返回 1 */
uint8_t VOFA_HasData(void);

/* 复制一行数据到输出缓冲区，清除就绪标志，返回实际复制的字符数 */
uint16_t VOFA_Read(char *output, uint16_t output_size);

/* 解析 "key:value" 格式行，键名匹配成功则将 value 转为浮点数，返回 1 */
uint8_t VOFA_ParseFloat(const char *expected_key, float *value);

#endif