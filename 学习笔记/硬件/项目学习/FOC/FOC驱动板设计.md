# 一、设计需求
该项目后续需要应用在桌面级轮腿机器人上，需要驱动2个无刷电机，要求驱动板可以支持两路驱动。目前计划使用2208无刷电机，额定12V，0.8A，即最后成品过流>=2A。为方便调试，需要板载一块0.78寸的屏幕。需要有足够丰富的外设资源：UART、IIC、CAN、SPI等。

# 二、同类设计分析
## （一）鲲FOC无刷电机控制器
开源地址：[鲲FOC无刷电机控制器（已适配灯哥开源FOC程序） - 立创开源硬件平台](https://oshwhub.com/expert/xin-shuang-lu-simplefoc-wu-shua-dian-ji-qu-dong-qi-yi-shi-pei-deng-ge-foc-cheng-xu)
尺寸为80mm x 45mm，主控采用ESP32Wroom，资源：两路无刷电机驱动，两路iic，一路can，一路usart接口电路带载能力：供电12V，输出最大电流2A。


![](assets/FOC驱动板设计/file-20261013085423355.png)
## （二）不知名网友的设计
获知地址：[网友设计的无刷电机驱动板，主控是STM32，让我帮他看下，历时三天出了这期分析视频_哔哩哔哩_bilibili](https://www.bilibili.com/video/BV1Xo4y1w7zH?spm_id_from=333.788.videopod.sections&vd_source=f463c94bb693bc70ae0b1979f04aa8aa)
