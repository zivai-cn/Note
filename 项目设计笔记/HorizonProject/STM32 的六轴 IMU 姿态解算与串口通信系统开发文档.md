
# 第 1 章　项目概述
## 1.1 项目背景
本项目基于stm32平台，设计并实现了对IMU的姿态解算以及上位机通信。本项目测试使用IMU为MPU6050。系统通过I2C获取六轴IMU的数据，包括三轴加速度计以及三轴陀螺仪数据，完成了对传感器数据的处理、零偏校准及姿态融合，最终通过串口输出四元数和欧拉角的数据包。
## 1.2 项目目标
1. 实现IMU的初始化、读取数据、异常检测。
2. 实现 IMU 的上电自校准和零漂抑制功能。
3. 实现对IMU数据的滤波。
4. 实现对IMU数据的姿态融合，解算四元数。
5. 建立1kHZ的数据采集与处理机制。
6. 设计简易的串口通信协议，向上位机传递数据。
7. 进行系统测试。

# 第 2 章　需求分析与指标分解

# 第 3 章　系统总体设计
# 第 4 章　硬件接口与底层驱动设计
## 4.1 MCU
芯片为STM32F407VET6，主频168MHZ，本项目工程采用STM32CUBEMX生成的基于HAL库，以FREERTOS为系统的源码。
## 4.2 IMU
芯片为MPU6050，通信协议采用I2C，设备ID：
第 5 章　IMU 数据采集与校准设计
第 6 章　姿态融合算法设计
第 7 章　实时调度与软件运行流程
第 8 章　自定义串口通信协议
第 9 章　异常处理与可靠性设计
第 10 章　软件工程实现说明
第 11 章　系统测试与结果分析
第 12 章　结论与改进方向


# 项目的初始化
1. RCC时钟源 设置，高速和低速时钟全部使用外部晶振。
![](assets/STM32%20的六轴%20IMU%20姿态解算与串口通信系统开发文档/file-20261014194319492.png)
2. SYS下，Debug模式设置为Serial Wire串行总线调试；由于项目要求使用Freertos，故给系统设置时基源，选择不常用的TIM14。
![](assets/STM32%20的六轴%20IMU%20姿态解算与串口通信系统开发文档/file-20261014194401170.png)
3. Freertos软件API标准选择为CMSIS_V2。
![](assets/STM32%20的六轴%20IMU%20姿态解算与串口通信系统开发文档/file-20261014195005178.png)
4. 开发环境选择为CMake
![](assets/STM32%20的六轴%20IMU%20姿态解算与串口通信系统开发文档/file-20261014195121152.png)

# 二、软件设计
## （一）工作流程
![](assets/STM32%20的六轴%20IMU%20姿态解算与串口通信系统开发文档/horizonproject.drawio.png)
## （二）任务拆解
### 任务一：读取IMU数据
[MPU-6050 -PDF数据手册-参考资料-立创商城](https://item.szlcsc.com/datasheet/MPU-6050/24852.html?spm=sc.gbn.xds.a___sc.gbn.hd.ss&c=BD&sdclkid=ALfi15fG15fNAJDib-&bd_vid=8182337746235296236&lcsc_vid=QwMKBgBfEwdWBlRXElgKUlVQEQNYAQVfE1dbVgUDFFYxVlNfRlhaU1FfT1BeVzsOAxUeFF5JWA8aF0oLBA9ABApJ)
本项目选择的IMU芯片为MPU6050，根据数据手册信息，该芯片采用IIC通信。


