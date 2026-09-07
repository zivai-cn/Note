# 一、原创作者
[wanshouxujiu](https://oshwhub.com/wanshouxujiu?jspm=hub.gc.yh.zy___hub.ss.lb.gc30&jlc_vid=TgVYBgFTFFMIVlFQRARdVQFWR1BfUFRUQlALUgIEQgMxVlNeT1ZYXlRfT1BbVjsOAxUeFF5JWA4dDxMOAgNABAsLWBIbFRQLGBUGAhYAAgIFAhZJEA4AAEkAFg8JSgYPFg5DHQwTGUwKDgUIWhgbBgo5GBIGEAwTFU8NCAk%3D)
本项目是对大佬的项目的分析学习。
# 二、器件选型与方案设计分析
## （一）电源树与布局
![](assets/基于stm32的数控电源/file-20260907202108626.png)
## （二）主控MCU
原作者的设计采用的是stm32f103c8t6芯片，但是经过我的考量，这个项目是一个不错的可以应用LVGL进行图形化显示的项目，而运行图形库f103的性能显然是不够的。于是看更高级的芯片，包括stm32f4、g4系列，