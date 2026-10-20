# 一、项目概述
本项目是基于stm32平台的通用开发架构模板。底层基于STM32HAL库，实现了对串口、SPI、FDCAN等常用通信协议的重封装，将底层调用与上层APP完全解耦，同时保留了统一的拓展接口以供二次开发。
# 二、参考项目分析
## 2.1 参考源码——basic_framework
本项目参考了湖南大学RoboMaster跃鹿战队的EC basic_framework-dev，开源仓库地址：[basic_framework: Hey this is the basic frame work for robomaster standard infantry Robots! enjoy using it, have fun developing Robot with us](https://gitee.com/hnuyuelurm/basic_framework)
## 2.2 整体层级
这是一个嵌入式通用的开发架构，为了保证“通用性”，在代码设计上，项目采用了在STM32CUBEMX初始化生成的文件的基础上进行拓展，将所有代码文件整体分为3类，分别为：BSP/底层算法、Modules/高级算法、Applications。每一层都只对外更高一层暴露自己的接口参数，对内隐藏自己调用的底层函数或者低层级的接口信息。通过设计的统一的函数接口参数样式，实现了即使更换底层文件，上层文件代码不需要作出任何修改。
## 2.3 通信协议的设计
通过定义一个通用的XXXInstance结构体，将一个通信实例的所有信息对外暴露，例如接收数据缓存区，回调函数指针，实例id等。
对外的所有函数设计为只有初始化函数，注册实例函数，接收信息函数，发送信息函数，中断回调函数，错误函数。
# 三、架构设计
## 3.1 USART的BSP封装
在原版的设计中，USART的BSP函数设计存在在短时间内连续调用此接口，若采用 IT/DMA 会导致上一次的发送未完成而新的发送取消的问题。因此在本项目中设计了环形队列，当

