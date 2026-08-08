# 一、项目文件架构
crazyflie作为一个成熟的项目，代码涉及的方面非常宽泛，对代码文件的分类非常重要。
```c项目架构
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
modules{//模块功能
//完整功能的最小单位
	system.c //系统任务 - 初始化所有模块并启动
	stabilizer.c //【关键】飞控稳定器任务 - 1kHz 主控制循环
	crtp.c //通信协议栈 (TX/RX 任务)
	log.c //日志系统任务
	param_task.c //参数系统任务
	commander.c //指令处理
	estimator/ //状态估计 (卡尔曼滤波器任务)
	controller/ //控制器 (PID/Mellinger/INDI 等)
}
deck{//扩展板驱动 (每个驱动可能创建任务)

}
drivers{//底层驱动
//传感器读取，电机控制，iic，spi等通信协议
	
}
utils{//工具函数

}
```

```c文件架构
platform{
	interface{//对外暴露的操作接口
		xxx.h
	}
	src{//核心代码文件
		xxx.c
	}
}
```
# 二、程序运行流程分析
## （一） system.c
代码负责整个系统的初始化、自检、启动流程。协调所有子模块的初始化顺序。
1. 系统启动函数`void systemLaunch(void)`
调用函数是freertos静态分配内存的重定义函数。
2. 第一次初始化`systemInit(void)`

# 三、附属小知识
### C 预处理器 ## 符号说明
`##` 是 C 预处理**令牌拼接运算符**：把前后两段标识符拼接成一个完整变量名。
举例：当传入`NAME = systemTask`
- `osSys_ ## NAME ## StackDepth` → `osSys_systemTaskStackDepth`
用途：对函数进行重封装，重定义传入参数名称。