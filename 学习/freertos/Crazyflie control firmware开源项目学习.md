# 一、项目架构
crazyflie作为一个成熟的项目，代码涉及的方面非常宽泛，对代码文件的分类非常重要。
```c
init{//项目的启动入口
	main.c//freertos的启动入口
}
config{//全局设置，包括freertos内核参数
	
}
hal{//(hardware abstract layer)硬件抽象层

}
```