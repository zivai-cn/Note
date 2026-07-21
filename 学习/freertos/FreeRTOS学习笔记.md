# 一、使用STM32CUBEMX创建freertos工程
1. 先按照正常的创建流程创建工程，不一样的是，配置系统时钟-调试接口：serial wire，基准时钟需要选择一个定时器TIMX。
![](assets/FreeRTOS学习笔记/file-20260721164234669.png)
2. 配置freertos相关设置。
![](assets/FreeRTOS学习笔记/file-20260721164321153.png)
![](assets/FreeRTOS学习笔记/file-20260721164340893.png)
![](assets/FreeRTOS学习笔记/file-20260721164510684.png)
# 二、创建一个freertos示例双任务程序
项目目标：使用freertos，实现两个完全不相关的任务的同时运行。
步骤：
1. 在freertos.c中创建一个初始化函数，默认创建了一个任务。