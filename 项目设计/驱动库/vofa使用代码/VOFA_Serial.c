#include "VOFA_Serial.h"
#include "Serial.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define VOFA_TX_BUFFER_SIZE  128U

/* 接收缓冲区：存储从串口逐字节接收的字符，直到收到换行符 */
static volatile char VOFA_RxBuffer[VOFA_RX_BUFFER_SIZE];
/* 当前已接收的行长度（不含结尾 '\0'） */
static volatile uint16_t VOFA_RxLength = 0U;
/* 接收就绪标志：当收到完整一行（以 '\n' 结尾）后置为 1 */
static volatile uint8_t VOFA_RxReady = 0U;

/*
 * VOFA_Init - 初始化 VOFA 串口通信
 *
 * 调用底层 Serial_Init 函数，使用配置的 USART 外设和波特率
 * 完成 GPIO、USART 时钟及外设的初始化工作
 */
void VOFA_Init(void)
{
    Serial_Init(VOFA_USART, VOFA_BAUDRATE);
}

/*
 * VOFA_SendString - 通过串口发送字符串
 *
 * 参数 text：要发送的 C 字符串指针
 *
 * 流程：
 *   1. 空指针检查：若 text 为 NULL 则直接返回
 *   2. 计算字符串长度后通过 Serial_SendArray 逐字节发送
 */
void VOFA_SendString(const char *text)
{
    if (text == NULL)
    {
        return;
    }

    Serial_SendArray(VOFA_USART,
                     (uint8_t *)text,
                     (uint16_t)strlen(text));
}

/*
 * VOFA_Printf - 格式化字符串并通过串口发送（类似 printf）
 *
 * 参数 format：格式化字符串，支持 printf 标准格式说明符
 * 参数 ...：可变参数列表，与 format 中的占位符一一对应
 *
 * 流程：
 *   1. 空指针检查
 *   2. 使用 vsnprintf 将格式化结果写入栈上的临时缓冲区 buffer
 *   3. 若格式化结果超出缓冲区大小，则截断以防止溢出
 *   4. 通过 Serial_SendArray 发送格式化后的字符串
 */
void VOFA_Printf(const char *format, ...)
{
    char buffer[VOFA_TX_BUFFER_SIZE];
    int length;
    va_list args;

    if (format == NULL)
    {
        return;
    }

    va_start(args, format);
    length = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (length <= 0)
    {
        return;
    }

    if (length >= (int)sizeof(buffer))
    {
        length = (int)sizeof(buffer) - 1;
    }

    Serial_SendArray(VOFA_USART, (uint8_t *)buffer, (uint16_t)length);
}

/*
 * VOFA_InputByte - 从串口接收中断中逐字节喂入数据
 *
 * 参数 byte：从 USART 数据寄存器读取到的单个字节
 *
 * 流程：
 *   1. 若上一行尚未被应用层读取（VOFA_RxReady == 1），则丢弃新数据
 *   2. 忽略回车符 '\r'（仅以换行符 '\n' 作为行结束标志）
 *   3. 收到 '\n' 时在缓冲区末尾添加 '\0'，并置位就绪标志
 *   4. 普通字符存入缓冲区；若缓冲区满则丢弃整行，防止溢出
 *
 * 注意：此函数应在 USART RX 中断服务函数中调用
 */
void VOFA_InputByte(uint8_t byte)
{
    /* 上一行尚未被读取，丢弃新数据 */
    if (VOFA_RxReady != 0U)
    {
        return;
    }

    /* 忽略回车符 */
    if (byte == (uint8_t)'\r')
    {
        return;
    }

    /* 收到换行符，一行接收完成 */
    if (byte == (uint8_t)'\n')
    {
        VOFA_RxBuffer[VOFA_RxLength] = '\0';
        VOFA_RxReady = 1U;
        return;
    }

    /* 缓冲区未满则存入字符 */
    if (VOFA_RxLength < (VOFA_RX_BUFFER_SIZE - 1U))
    {
        VOFA_RxBuffer[VOFA_RxLength++] = (char)byte;
    }
    else
    {
        /* 缓冲区已满，丢弃当前超长行 */
        VOFA_RxLength = 0U;
    }
}

/*
 * VOFA_HasData - 查询是否收到完整的一行数据
 *
 * 返回值：1 表示有完整行等待读取，0 表示尚无数据
 *
 * 典型用法：在主循环中轮询此函数，返回 1 时调用 VOFA_Read 读取
 */
uint8_t VOFA_HasData(void)
{
    return VOFA_RxReady;
}

/*
 * VOFA_Read - 读取已接收的一行数据
 *
 * 参数 output：输出缓冲区指针，用于存放读取到的字符串
 * 参数 output_size：输出缓冲区大小（字节数）
 *
 * 返回值：实际复制的字符数（不含结尾 '\0'），若未就绪或参数无效则返回 0
 *
 * 流程：
 *   1. 校验参数有效性及就绪状态
 *   2. 计算实际可复制的长度（不超过 output_size - 1，预留 '\0' 空间）
 *   3. 逐字节复制到 output 缓冲区，末尾添加 '\0'
 *   4. 清除接收状态，准备接收下一行
 */
uint16_t VOFA_Read(char *output, uint16_t output_size)
{
    uint16_t index;
    uint16_t copy_length;

    if ((output == NULL) || (output_size == 0U) || (VOFA_RxReady == 0U))
    {
        return 0U;
    }

    copy_length = VOFA_RxLength;
    if (copy_length >= output_size)
    {
        copy_length = output_size - 1U;
    }

    for (index = 0U; index < copy_length; index++)
    {
        output[index] = VOFA_RxBuffer[index];
    }
    output[copy_length] = '\0';

    VOFA_RxLength = 0U;
    VOFA_RxReady = 0U;
    return copy_length;
}

/*
 * VOFA_ParseFloat - 解析 "key:value" 格式的字符串，将 value 转为浮点数
 *
 * 参数 expected_key：期望匹配的键名（如 "first_Kp"）
 * 参数 value：输出参数，解析成功时写入浮点数值
 *
 * 返回值：1 表示匹配成功并解析，0 表示失败
 *
 * 流程：
 *   1. 参数校验：检查 expected_key、value 是否为空，数据是否就绪
 *   2. 查找冒号 ':' 分隔符，未找到则丢弃此行并返回 0
 *   3. 提取冒号前的键名，与 expected_key 比较（长度和内容均需匹配）
 *   4. 匹配成功则将冒号后的字符串通过 strtof 转为浮点数
 *   5. 清除接收状态，准备接收下一行
 *
 * 示例：输入 "first_Kp:6.39\n"
 *       调用 VOFA_ParseFloat("first_Kp", &val) → val = 6.39f, 返回 1
 */
uint8_t VOFA_ParseFloat(const char *expected_key, float *value)
{
    const char *colon;
    uint16_t key_len;
    uint16_t expected_len;
    char *endptr;

    if ((expected_key == NULL) || (value == NULL) || (VOFA_RxReady == 0U))
    {
        return 0U;
    }

    /* 查找冒号分隔符 */
    colon = strchr((const char *)VOFA_RxBuffer, ':');
    if (colon == NULL)
    {
        /* 格式错误，丢弃此行 */
        VOFA_RxLength = 0U;
        VOFA_RxReady = 0U;
        return 0U;
    }

    /* 计算键名长度并与期望键名比较 */
    key_len = (uint16_t)(colon - (const char *)VOFA_RxBuffer);
    expected_len = (uint16_t)strlen(expected_key);

    if ((key_len != expected_len) ||
        (strncmp((const char *)VOFA_RxBuffer, expected_key, key_len) != 0))
    {
        /* 键名不匹配，丢弃此行 */
        VOFA_RxLength = 0U;
        VOFA_RxReady = 0U;
        return 0U;
    }

    /* 解析冒号后的数值 */
    *value = strtof(colon + 1, &endptr);

    /* 清除接收状态 */
    VOFA_RxLength = 0U;
    VOFA_RxReady = 0U;

    return 1U;
}