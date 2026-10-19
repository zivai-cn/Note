# 一、项目概述
本项目是基于stm32平台的通用开发架构模板。底层基于STM32HAL库，实现了对串口、SPI、FDCAN等常用通信协议的重封装，将底层调用与上层APP完全解耦，同时保留了统一的拓展接口以供二次开发。
# 二、参考项目分析
## 2.1 参考源码——basic_framework
本项目参考了湖南大学RoboMaster跃鹿战队的EC basic_framework-dev，开源仓库地址：[basic_framework: Hey this is the basic frame work for robomaster standard infantry Robots! enjoy using it, have fun developing Robot with us](https://gitee.com/hnuyuelurm/basic_framework)
## 2.2 整体层级
这是一个嵌入式通用的开发架构，为了保证“通用性”，在代码设计上，项目采用了在STM32CUBEMX初始化生成的文件的基础上进行拓展，将所有代码文件整体分为3类，分别为：BSP/底层算法、Modules/高级算法、Applications。每一层都只对外更高一层暴露自己的接口参数，对内隐藏自己调用的底层函数或者低层级的接口信息。通过设计的统一的函数接口参数样式，实现了即使更换底层文件，上层文件代码不需要作出任何修改。
# 三、架构设计

