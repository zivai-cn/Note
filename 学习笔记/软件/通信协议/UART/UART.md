# 概述
UART，全称：Universal Asynchronous Receiver Transmitte，通用异步收发传输器。是一种全双工、异步的通信协议，特点是点对点。物理接线示例如下：
![570](assets/UART/file-20260831164942336.png)
# 物理结构
常见有四根导线，vcc（电源正极）,txd（发送端）,rxd（接收端）,gnd（电源负极）。**注意**TX与RX需要交叉连接。
因为没有统一的时钟线，即异步通信，因此必须要共地，提供相同的电压基准。
# 帧格式
- 起始位：拉低 1 bit 时间，告诉对方"我要发数据了"
- 数据位：5~9 位，LSB 先发（最低位先发）
- 校验位（可选）：奇校验或偶校验
- 停止位：拉高 1~2 bit 时间，表示一帧结束
# 需要设置的参数
波特率：每秒传输的bit数量。常见有9600、115200等。
数据位：每帧中，有效数据的位数。最常用为8位。
停止位：一帧结束的标志。
起始位：一帧开始的标志。常用的设计方式是起始位为低电平，停止位为高电平。在默认状态下数据线上保持高电平，起始位设计为拉低电平，实际的检测是检测到一个下降边沿，再检测到一个低电平来确认的起始；结束位的判定就是简单的在数据传输的末尾时间片中检测到高电平就认为是停止位。
# 适用场景
通信结构的设计就是点对点通信，所以通常用在简单电路中。
常见的使用地点是调试输出，上下位机的通信。
# USART
通用同步异步收发传输器。
加入了同步功能，在物理接线上引入了一根时钟线。
基本从不会用同步功能。
# 在STM32上使用UART
UART一般用作传输固定格式的数据，因此，在stm32上有三种使用方式：1.轮询模式 2.中断模式 3.DMA模式
### (一)初始化
```c
// 1. 声明句柄
UART_HandleTypeDef huart1;
// 2. 配置参数（最常用的 115200-8-N-1）
huart1.Instance          = USART1;
huart1.Init.BaudRate     = 115200;
huart1.Init.WordLength   = UART_WORDLENGTH_8B;
huart1.Init.StopBits     = UART_STOPBITS_1;
huart1.Init.Parity       = UART_PARITY_NONE;
huart1.Init.Mode         = UART_MODE_TX_RX;
huart1.Init.OverSampling = UART_OVERSAMPLING_16;
// 3. 初始化
HAL_UART_Init(&huart1);
```
### （二）发送
```c
// ① 阻塞发送 —— 最简单，会卡住 CPU 直到发完
uint8_t data[] = "Hello\r\n";
HAL_UART_Transmit(&huart1, data, sizeof(data)-1, 1000);  // 超时 1000ms
// ② 中断发送 —— 不阻塞，发完触发回调
HAL_UART_Transmit_IT(&huart1, data, sizeof(data)-1);
// 发完后自动调用 HAL_UART_TxCpltCallback() 
// ③ DMA 发送 —— 完全不占 CPU，适合大数据量
HAL_UART_Transmit_DMA(&huart1, data, sizeof(data)-1);
// 发完后自动调用 HAL_UART_TxCpltCallback()
```
### （三）接收
```c
uint8_t rx_buf[1];
// ① 阻塞接收 —— 卡住等数据
HAL_UART_Receive(&huart1, rx_buf, 1, 1000);
// ② 中断接收 —— 最常用！每收到 1 字节触发回调
HAL_UART_Receive_IT(&huart1, rx_buf, 1);
// 收到后自动调用 HAL_UART_RxCpltCallback()
// ③ DMA 空闲中断 —— 接收不定长数据的利器
HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rx_buf, BUF_SIZE);
// 收到数据或检测到空闲帧后触发 HAL_UARTEx_RxEventCallback()
```
### （四）重定向
```c
// 重定向 fputc，让 printf 通过串口输出
int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 10);
    return ch;
}
// 然后就可以愉快地使用了
printf("ADC Value: %d\r\n", adc_value);
printf("System Tick: %lu\r\n", HAL_GetTick());
```
### （五）中断回调函数的使用
```c
// 接收中断回调
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        // 处理刚收到的 1 字节
        uint8_t byte = rx_buffer[0];
        
        // 简单回显
        HAL_UART_Transmit(&huart1, &byte, 1, 10);
        
        // 再次开启接收中断（重要！否则只收一次）
        HAL_UART_Receive_IT(&huart1, rx_buffer, 1);
    }
}
```
