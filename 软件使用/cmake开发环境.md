# 一、笔记介绍
本笔记内容来自bilibili网站up主ControlCoreX。主要讲解了在vscode中使用cmake进行stm32开发的环境配置。
随笔记附赠一份模板工程，同样来自up主。链接如下：
# 二、介绍vscode
1. workspace
在vscode中打开一个文件夹，软件就默认加载这个文件夹为workspace，会在该文件夹下生成一个workspace.json。
workspace就相当于其他编译器如keil的建立的工程项目。
2. 设置
vscode提供了ui界面进行设置，同时也可以通过settings.json进行设置，在后续的操作中，需要通过settings.json进行设置。
settings的级别：settings有两个，分别是workspace的settings还有user的settings。user的settings是全局设置，workspace的settings针对的是这个工作文件，移植到其他项目的文件夹下同样会生效。储存地址是workspace/.vscode/settings.json。workspace的settings优先级要高于user。
3. task
通过task，开发者可以直接使用命令行指令，包括make，ninja，cmake等，无需打开终端进行操作。
task的配置文件为tasks.json，地址是workspace/.vscode/tasks.json
# 三、模板工程的使用
## （一）安装工具链
1. 下载工具链，建议放在d盘的一个专门用来放置环境相关文件的地方。包括：cmake，ninja，arm-gnu-toolchain，openocd，clangd
2. 将工具链添加进入系统环境变量。如何设置环境变量，请看自行查找。
![494](assets/cmake开发环境/file-20260716210952890.png)
3. 