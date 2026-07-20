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
3. 添加完成测试，在终端中查询版本号。如果可以出现版本号，证明前面步骤都是正确的。
```c
cmake --version
cmake version 3.28.1

ninja --version
1.11.1

arm-none-eabi-gcc --version
arm-none-eabi-gcc.exe (GNU Tools for STM32 13.3.rel1.20250523-0900) 13.3.1 20240614
Copyright (C) 2023 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

openocd.exe --version
Open On-Chip Debugger 0.12.0 (2026-03-02) [https://github.com/sysprogs/openocd]
Licensed under GNU GPL v2
libusb1 d52e355daa09f17ce64819122cb067b8a2ee0d4b
For bug reports, read
        http://openocd.org/doc/doxygen/bugs.html

clangd.exe --version
clangd version 22.1.8 (https://github.com/llvm/llvm-project ca7933e47d3a3451d81e72ac174dcb5aa28b59d1)
Features: windows
Platform: x86_64-pc-windows-msvc
```
4. 添加新的环境变量，openocd，添加到与path平级的系统变量。
![541](assets/cmake开发环境/file-20260716212802295.png)
## （二）vscode拓展
- clangd
- CMake Language Support
- Cortex-Debug
- LinkerScript
- Material Icon Theme
## （三）模板工程
适配芯片：STM32F407ZGT6
DEMO链接: https://pan.baidu.com/s/14EFUeQPD89I2WextZFJsxg?pwd=nbmp 提取码: nbmp
# 四、详解cmake-ninja
## （一）构建系统的层次
- 实际中项目的构建最底层是在编译器，但是直接操作编译器进行代码的开发非常麻烦，为了解决这个问题，创造了make工具。但是makefile的编写语法有些抽象，可读性比较差，因此就开发了cmake工具（cmakelist）。cmake工具是在make的更上一层的工具。
- 构建层次：cmake->make->编译器
## （二）MinGW
MinGW全称Minimalist GNU for Windows。GNU编译器只能在Linux上运行，通过MinGW，可以实现在Windows PC上运行编译器。此外还有arm编译器等，只能在单片机上运行。
MinGW中集成了make工具，在我们的开发环境中，下载MinGW实际上也就是为了使用这个make工具。
## （三）ninja
Ninja工具与make工具平级，Ninja的代码语法极其抽象，开发者难以阅读，因此必须依赖更高层级的工具的调用才能使用，相比之下，make工具语法会平易近人许多，所以说make可以直接使用。
- Ninja的设计是为了方便电脑阅读，设计的核心目的是为了更快的编译速度。
## （四）交叉编译器
在A上运行的编译器，编译出的结果文件运行在B平台上，这个编译器就是交叉编译器。例如：arm编译器。
## （五）clangd（llvm）
clangd是一个语法高亮提示工具，如果需要使用需要前后端两个部分。后端需要在工具链中下载clangd文件，前端需要在编译界面软件中添加插件。
# 五、STM32工程结构
# 六、cubemx的cmake移植
这里我以stm32f4xx芯片举例，在下文中，我会指出，不同的芯片具体应该如何进行配置。
1. cubemx的配置
- 在project Manager中，设置toolchain/IDE为cmake，Default Compiler/Linker为GCC。
2. 了解cubumx生成hal库代码的文件结构
- 