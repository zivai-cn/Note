# 一、项目架构
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