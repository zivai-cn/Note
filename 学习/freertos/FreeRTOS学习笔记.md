学习笔记来自00kino，学习老师包括：伟东山 bilibili，keysking bilibili。
**学前知识：**
- 由于FreeRtos工程的自身特性，在freertos的学习过程中，你会看到两套API函数，一个来自Freertos，一个来自CMSIS，这里00kino认为，FreeRtos的操作更加底层，因此笔记中使用的都是FreeRtos的原生API，但是CUBEMX生成的代码默认使用的都是CMSIS的API，需要注意。
- 在FreeRtos中，很多函数是宏定义的类函数，这意味着如果配置不正确，你可能无法跳转到相关定义，此时你可以直接查找 `#define 函数`
# 一、使用STM32CUBEMX创建freertos工程
1. 先按照正常的创建流程创建工程，不一样的是，配置系统时钟-调试接口：serial wire，基准时钟需要选择一个定时器TIMX。
![](assets/FreeRTOS学习笔记/file-20260721164234669.png)
2. 配置freertos相关设置。
![](assets/FreeRTOS学习笔记/file-20260721164321153.png)
![](assets/FreeRTOS学习笔记/file-20260721164340893.png)
![](assets/FreeRTOS学习笔记/file-20260721164510684.png)
# 二、任务系统的创建与调度
项目目标：能够使用freertos创建任务。
1. 在freertos.c中创建一个初始化函数，默认创建了一个任务：defaultTaskHandle。
![](assets/FreeRTOS学习笔记/file-20260721171444987.png)
2. 谈CMSIS。
众所周知，stm32芯片的内核是由arm公司设计的，CMSIS就是arm官方编写的芯片内核调用的一套标准化软件接口规范，包括底层驱动文件软件架构、头文件、库模板。CMSIS为所有arm设计内核都提供了一套统一接口函数，如果需要编辑芯片内部的工作，只需要调用CMSIS的接口函数即可。RTOS本质就是一个系统内核调度系统，它的工作就是需要和芯片内核进行交互。目前，有很多的RTOS，例如FreeRTOS，RTThread等，他们各自有一套自己的系统调度函数，用法各不相同。为了保证使用的方便，在这些RTOS设计时，就保证了函数可以对接CMSIS的接口函数，进而完成相应功能。最终：你的代码 → CMSIS-RTOS2 → FreeRTOS → 硬件
3. 整个任务的函数调用流程。
以defaultTaskHandle为例，defaultTaskHandle<--osThreadNew(CMSIS文件)，其中osThreadNew是对xTaskCreate()的一层封装。
4. 关键函数与结构体的解释
```c
/// Create a thread and add it to Active Threads.
/// \param[in]     func          thread function.执行功能的函数
/// \param[in]     argument      pointer that is passed to the thread function as start argument.给功能函数传递的参数
/// \param[in]     attr          thread attributes; NULL: default values.任务属性结构体
/// \return thread ID for reference by other functions or NULL in case of error.返回值：如果执行成功：返回任务ID，失败：返回NULL
osThreadId_t osThreadNew (osThreadFunc_t func, void *argument, const osThreadAttr_t *attr);

// osThreadAttr_t 的定义（在 cmsis_os2.h 中） 
//常用字段：
|name|任务名（调试用）|"LED_Task"|
|stack_size|栈大小（**字节**，注意！）|512|
|priority|优先级|osPriorityNormal|
typedef struct 
{ const char *name; // 任务名称（调试用） 
uint32_t attr_bits; // 属性位（如 osThreadDetached） 
void *cb_mem; // TCB 内存（通常 NULL，让系统分配） 
uint32_t cb_size; // TCB 大小（通常 0） v
oid *stack_mem; // 栈内存（通常 NULL，让系统分配） 
uint32_t stack_size; // 栈大小（单位：字节）★ 
osPriority_t priority; // 优先级（osPriorityLow ~ osPriorityRealtime7）
TrustZone_ID tz_module; // TrustZone 相关（通常 0） 
uint32_t reserved; // 保留 
} osThreadAttr_t;

//FreeRTOS 原生的任务创建函数
BaseType_t xTaskCreate(
    TaskFunction_t              pxTaskCode,     // ① 任务函数
    const char * const          pcName,         // ② 任务名称
    const configSTACK_DEPTH_TYPE usStackDepth,  // ③ 栈深度
    void * const                pvParameters,   // ④ 任务参数(给任务函数的传参)
    UBaseType_t                 uxPriority,     // ⑤ 优先级
    TaskHandle_t * const        pxCreatedTask   // ⑥ 任务句柄（输出）
);

//将时间转换成系统节拍拍数函数
pdMS_TO_TICKS();      //参数为时间（单位ms）

//延时函数
vTaskDelay(pdMS_TO_TICKS(100));       //相对延时，从延时函数开始工作时刻起等100ms，无法保证什么时候跳出。
vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100)); //绝对延时，保证整个任务周期性清醒。
```
# 三、任务状态机
目标：了解任务的几种状态，状态之间是如何进行交换的。能够使用操纵任务状态的几个函数。
1. 任务状态：（1）ready，就绪态。创建的任务首先进入ready，之后，只要任务没有被执行，就会进入就绪态。（2）running，运行态。时间片流转，任务执行。（3）blocked，阻塞态。当任务被延时，或者设置后需要接收数据才能继续运行，任务就会blocked（4）suspended，挂起态。程序可以通过函数要求任务suspended，此时任务将不会再进入ready，直到再通过函数，让任务跳出suspended，进入ready。
![](assets/FreeRTOS学习笔记/file-20260723160951848.png)
2. 关键函数
```c
//任务的挂起与恢复
vTaskSuspend(xTaskHandle);           // 暂停某个任务
vTaskResume(xTaskHandle);            // 恢复某个任务
vTaskSuspendAll();                   // 暂停调度器（临界区用）
xTaskResumeAll();                    // 恢复调度器

// 删除任务
vTaskDelete(NULL);                   // 删除自己
vTaskDelete(xTaskHandle);            // 删除别人

// 优先级操作
uxTaskPriorityGet(NULL);             // 获取自己优先级
vTaskPrioritySet(NULL, 3);           // 修改自己优先级
```
3. 调试
Freertos创建任务后，会默认直接把优先级最大的任务直接挂到运行态。
4. 应用
```c
//使用暂停恢复调度器，对运算进行保护。
// ========== 加保护（安全） ========== 
vTaskSuspendAll(); // 暂停调度，谁都别想切进来 
X = 100; 
Y = 100; // 这两行一定连续执行完 
xTaskResumeAll(); // 恢复调度

```
# 四、队列
目标：理解队列的运行，了解freertos中队列的相关函数。
1. IPC机制
inter  process communication，进程间通信，也指任务间通信，用于不同任务/进程之间的数据传输与同步的机制。
```
  发送方任务               队列缓冲区              接收方任务
  ┌────────┐      ┌───┬───┬───┬───┬───┐      ┌────────┐
  │ Task A │ ───→ │ 1 │ 2 │ 3 │   │   │ ───→ │ Task B │
  └────────┘      └───┴───┴───┴───┴───┘      └────────┘
                        ↑    ↑    ↑
                      入队   队列   出队
                           满则阻塞
```
2. Queue队列
队列是freertos中最常见的IPC机制，具体是：创建一个链表或数组，A任务可以向其中写入数据，B任务可以读取其中的数据。其中A任务称作生产者，B任务称作消费者。
3. FIFO缓冲区
First In, First Out Buffer，先入先出。具体原理以及相关操作，请在数据结构与算法中学习，这里只需要知道，Queue就是一个典型的FIFO。首先被写入的数据，也会被首先读取出来。
4. 队列相关函数
```c
// 创建队列
//动态队列创建-内核从堆上分配内存
QueueHandle_t xQueue;//命名句柄
xQueue = xQueueCreate(
	uxQueueLength,   // 队列能容纳多少个元素
    uxItemSize       // 每个元素的大小（字节），用 sizeof(xxx)
);
//静态队列创建-自己规定内存
static QueueHandle_t xMyQueue;//命名句柄
static uint8_t ucQueueStorage[QUEUE_LENGTH * ITEM_SIZE];// 数据区 
static StaticQueue_t xQueueControlBlock; // 控制块，存储内核管理任务所需要的数据。
xMyQueue = xQueueCreateStatic(
	QUEUE_LENGTH, 
	ITEM_SIZE, 
	ucQueueStorage, // 指定数据存储区 
	&xQueueControlBlock // 指定控制块 
);

// 发送（三种方式）
xQueueSend(xQueue, &data, portMAX_DELAY);// 队列满则阻塞。
//该函数有返回值，如果data队列已满，返回pdPASS。
//如果队列有空间同时一直有数据传进来，那么它就会一直往队列里塞数据和portMAX_DELAY没有关系，portMAX_DELAY规定的是多长时间检查一次队列是否填满。
xQueueSendToBack(xQueue, &data, 0);// 从尾部入队（= Send）
xQueueSendToFront(xQueue, &data, 0);// 从头部入队（紧急数据）

// 接收
xQueueReceive(xQueue, &data, portMAX_DELAY);// 队列空则阻塞
//去队列里拿数据，如果没有数据，就睡觉等着，但最多等portMAX_DELAY，然后就会返回error，同时再次进入等待。如果一直有数据进入，那就会一直接收。

// 查询
uxQueueMessagesWaiting(xQueue);// 队列中还有多少数据
uxQueueSpacesAvailable(xQueue);// 队列还有多少空位
```
5. 应用
```c
//一、多生产者-单消费者
//注：因为是多生产者，因此在向消费者传递消息的时候，需要标注信息来源。
typedef struct {//消息结构体
int32_t producer_id;//ID
int32_t value; //数据
} Msg_t;
void vProducer(void *pv){//生产者任务
	int32_t id = (int32_t)pv;//定义消息结构体，初始化值
	Msg_t msg;
	msg.producer_id = id;
	msg.value=0;
	for(;;){
		//**任务代码**
		xQueueSend(xQuenue,&msg,portMAX_DELAY);//向队列传递信息
	}
}

//二、传递带时间戳的消息结构体
typedef struct {
	int32_t data;
	TickType_t timestamp//时钟tick
}TimeMsg_t;
void vProducer(void *pv){
	TimeMsg_t msg;
	mag.data = 0;
	for(;;){
		//**任务代码**
		mag.timestamp = xTaskGetTickCount();//打上时间戳
	}
}

//三、消息插队
//原本的消息是按照顺序进入队列，后入后出，但是紧急消息需要首先处理，需要被排在首位
typedef enum {//对消息进行分类
MSG_NORMAL = 0, 
MSG_URGENT = 1 
} MsgType_t;
void SendUrgent(int32_t val){//发送紧急消息
	Msg_t msg = { MSG_URGENT, val }; 
	xQueueSendToFront(xQueue, &msg, 0);//插到队首 
}
void vConsumer(void *pv) {//消费者优先处理紧急消息函数 
	Msg_t msg; 
	for (;;) { 
		if (xQueueReceive(xQueue, &msg, portMAX_DELAY) == pdPASS) { 
			if (msg.type == MSG_URGENT) { 
				printf("⚠ URGENT: %ld (processed FIRST!)\r\n", msg.value);
			} 
			else { 
				printf(" Normal: %ld\r\n", msg.value); 
			} 
			vTaskDelay(pdMS_TO_TICKS(200)); // 模拟处理耗时
		} 
	}
}
```
# 五、信号量
1. 二进制信号量
- 问题：如果需要一个事件的状态影响另一个事件的工作，可以直接传递信息也就是使用队列，但是我们只需要告知一个状态即可，设计一个队列未免有点资源浪费。
- 解决方案：二进制信号量，本质是一个单位的队列，只传递状态0/1。
- 用途：可以让A任务告知B任务自己的状态，或者用于中断响应。
- API函数
```c
//创建二进制信号量
SemaphoreHandle_t xSemaphore;
xSemaphore = xSemaphoreCreateBinary();
//释放信号量（将信号量置1）
xSemaphoreGive(xSemaphore);     
//获取信号量（将信号量置0）     
xSemaphoreTake(xBinarySem, portMAX_DELAY);  
```
2. 计数信号量
- 问题：存在多个任务竞争使用相同的资源，不能清晰表示剩余资源数量。
- 解决方案：还是使用一个队列，数据是对整体资源数量的标记值，每次使用、释放都对资源数进行重新记录。
- 用途：建立一个资源池，方便管理多个资源。
- API函数
```C
//创建计数信号量
SemaphoreHandle_t xSemaphore;
xSemaphore = xSemaphoreCreateCounting(max, init);//参数：资源最大值，初始被占用值
//消耗一个资源
xSemaphoreTake(xSemaphore,xBlockTime);//xSemaphore是等待时间  
//归还一个资源        
xSemaphoreGive(xSemaphore);             
```
3. 互斥信号量
- 问题：优先级反转——当多个优先级不同的任务都调用一个资源时，会出现下面的情况：存在三个优先级的任务H、M、L，其中M的执行不需要信号。H获取信号量，运行，释放信号量，接着信号被M、L执行。此时如果L任务的运行时间>H的运行时间，也就是说L没有释放信号量，此时H任务会尝试获取信号量，最终失败，接着就是M运行，也就是说，优先级低的任务反而阻塞了高优先级的任务。
- 解决方案：设计一个带有优先级继承机制的二进制信号量。当检测到H任务因为没有信号量而被L任务阻塞时，会将L的优先级临时提升到和H相同，L结束后直接执行H，将不会导致任务M优先执行。
- 用途：保护高优先级任务。
- API函数
```c
/*锁针对L*/
//创建互斥锁
SemaphoreHandle_t MutexSemaphore;
MutexSemaphore = xSemaphoreCreateMutex();
//加锁
xSemaphoreTake(xMutex, portMAX_DELAY);  
//解锁
xSemaphoreGive(xMutex);                 
```
4. FromISR
from interrupt service routine。
- 解决问题：中断函数中，不允许被阻塞，必须立刻返回。
- API函数
```c
//二进制信号量
xSemaphoreTakeFromISR();
xSemaphoreGiveFromISR();
//计数信号量
xSemaphoreTakeFromISR();
xSemaphoreGiveFromISR();
//互斥信号量没有FROMISR版本
```
4. 应用
```c
//按键中断唤醒任务
SemaphoreHandle_t xButtonSem;
void EXTI0_IRQHandler(void) {//中断处理函数中要用FromISR版本
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;//默认不切换
    xSemaphoreGiveFromISR(xButtonSem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);//如果需要切换就立即切换
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}	
void vButtonTask(void *pv) {
    for (;;) { 
        if (xSemaphoreTake(xButtonSem, portMAX_DELAY) == pdTRUE) {// 等待按键信号量，永远等
            printf("Button pressed!\r\n");
            // 处理按键逻辑...
        }
    }
}

//互斥锁保护串口打印
SemaphoreHandle_t xUARTMutex;
void vTask1(void *pv) {
    for (;;) {
        xSemaphoreTake(xUARTMutex, portMAX_DELAY);
        printf("Task1: This is a long message that must not be interrupted\r\n");
        xSemaphoreGive(xUARTMutex);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
void vTask2(void *pv) {
    for (;;) {
        xSemaphoreTake(xUARTMutex, portMAX_DELAY);
        printf("Task2: Another long message here\r\n");
        xSemaphoreGive(xUARTMutex);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
```
# 六、队列集
- 问题：队列只能在两个任务之间传递一种类型的信息，但有时两个任务之间交流的信息的更多的，比如需要传递数据，需要确认事件。
- 解决：为两个任务添加多个队列，组成一个队列集。
- 用途：让一个任务同时检查队列集中多个队列的信息，根据反馈执行操作。
1. API函数
```c
//创建队列集
QueueSetHandle_t queueset_handle;
xQueueCreateSet();
//添加队列到队列集
xQueueAddSet();
//删除队列集中队列
xQueueRemoveFromSet();
//获取队列集中获取消息的队列
xQueueSelectFromSet();
//中断中获取队列集中获取消息的队列
xQueueSelectFromSetFromISR();
```