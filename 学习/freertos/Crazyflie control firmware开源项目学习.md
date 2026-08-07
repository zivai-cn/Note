# 一、项目架构
crazyflie作为一个成熟的项目，代码涉及的方面非常宽泛，对代码文件的分类非常重要。
```c
init{//项目的启动入口
	main.c//freertos的启动入口
}
config{//全局设置，包括freertos内核参数
	
}
platform{//硬件平台抽象层
//板级资源调用
	platform.c//平台初始化
}
hal{//(hardware abstract layer)硬件抽象层
//外设资源调用
	syslink.c //STM32↔NRF51 通信任务
	usblink.c //USB 通信任务
	sensors_*.c //传感器数据采集任务
}

```