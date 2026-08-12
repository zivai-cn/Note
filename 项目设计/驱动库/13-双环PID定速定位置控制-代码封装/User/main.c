#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "LED.h"
#include "Timer.h"
#include "Key.h"
#include "RP.h"
#include "Motor.h"
#include "Encoder.h"
#include "Serial.h"
#include "PID.h"

/* 控制周期相关宏（Timer_Init 已设置 1ms 定时中断）
	- CTRL_TICK_MS: 定时器中断周期，单位 ms
	- INNER_PERIOD_MS: 内环执行周期，单位 ms（速度环，频率高）
	- OUTER_PERIOD_MS: 外环执行周期，单位 ms（位置环，频率低）
	可根据实际需要调整 INNER/OUTER 周期；通常内环比外环快数倍。 */
#define CTRL_TICK_MS       1
#define INNER_PERIOD_MS   40
#define OUTER_PERIOD_MS  200
uint8_t KeyNum;

int16_t Speed, Location;		//速度，位置

/*定义PID结构体变量*/
PID_t Inner = {					//内环PID结构体变量，定义的时候同时给部分成员赋初值
	.Kp = 0.3,					//比例项权重
	.Ki = 0.3,					//积分项权重
	.Kd = 0,					//微分项权重
	.OutMax = 100,				//输出限幅的最大值
	.OutMin = -100,				//输出限幅的最小值
};

PID_t Outer = {					//外环PID结构体变量，定义的时候同时给部分成员赋初值
	.Kp = 0.3,					//比例项权重
	.Ki = 0,					//积分项权重
	.Kd = 0.4,					//微分项权重
	.OutMax = 20,				//输出限幅的最大值
	.OutMin = -20,				//输出限幅的最小值
};

int main(void)
{
	/*模块初始化*/
	OLED_Init();		//OLED初始化
	Key_Init();			//非阻塞式按键初始化
	Motor_Init();		//电机初始化
	Encoder_Init();		//编码器初始化
	RP_Init();			//电位器旋钮初始化
	Serial_Init();		//串口初始化，波特率9600
	
	Timer_Init();		//定时器初始化，定时中断时间1ms

	PID_Init(&Inner);	//PID初始化，清零内部状态（增益与限幅保持初始化列表中的值）
	PID_Init(&Outer);	//PID初始化，清零内部状态

	/*OLED打印一个标题*/
	OLED_Printf(0, 0, OLED_8X16, "2*PID Control");
	OLED_Update();
	
	while (1)
	{
		/*按键修改目标值*/
		/*解除以下注释后，记得屏蔽电位器旋钮修改目标值的代码*/
		/*调节内环目标值使用Inner.Target，调节外环目标值使用Outer.Target*/
//		KeyNum = Key_GetNum();		//获取键码
//		if (KeyNum == 1)			//如果K1按下
//		{
//			Inner.Target += 10;		//目标值加10
//		}
//		if (KeyNum == 2)			//如果K2按下
//		{
//			Inner.Target -= 10;		//目标值减10
//		}
//		if (KeyNum == 3)			//如果K3按下
//		{
//			Inner.Target = 0;		//目标值归0
//		}
		
		/*解除下面一段代码的注释，进行内环PID调参*/
		/*进行内环PID调参时，请注释掉外环控制内环的部分代码*/
		/*RP_GetValue函数返回电位器旋钮的AD值，范围：0~4095*/
		/* 除4095.0可以把AD值归一化，再乘上一个系数，可以调整到一个合适的范围*/
//		Inner.Kp = RP_GetValue(1) / 4095.0 * 2;				//修改Kp，调整范围：0~2
//		Inner.Ki = RP_GetValue(2) / 4095.0 * 2;				//修改Ki，调整范围：0~2
//		Inner.Kd = RP_GetValue(3) / 4095.0 * 2;				//修改Kd，调整范围：0~2
//		Inner.Target = RP_GetValue(4) / 4095.0 * 300 - 150;	//修改目标值，调整范围：-150~150
//		
//		/*OLED显示*/
//		OLED_Printf(0, 16, OLED_8X16, "Kp:%4.2f", Inner.Kp);			//显示Kp
//		OLED_Printf(0, 32, OLED_8X16, "Ki:%4.2f", Inner.Ki);			//显示Ki
//		OLED_Printf(0, 48, OLED_8X16, "Kd:%4.2f", Inner.Kd);			//显示Kd
//		
//		OLED_Printf(64, 16, OLED_8X16, "Tar:%+04.0f", Inner.Target);	//显示目标值
//		OLED_Printf(64, 32, OLED_8X16, "Act:%+04.0f", Inner.Actual);	//显示实际值
//		OLED_Printf(64, 48, OLED_8X16, "Out:%+04.0f", Inner.Out);		//显示输出值
//		
//		OLED_Update();	//OLED更新，调用显示函数后必须调用此函数更新，否则显示的内容不会更新到OLED上
//		
//		Serial_Printf("%f,%f,%f\r\n", Inner.Target, Inner.Actual, Inner.Out);	//串口打印目标值、实际值和输出值
//																	//配合SerialPlot绘图软件，可以显示数据的波形
		
		
		/*解除下面一段代码的注释，进行外环PID调参*/
		/*内环PID调参完成后，加上外环控制内环的部分代码，再进行外环PID调参*/
		/*RP_GetValue函数返回电位器旋钮的AD值，范围：0~4095*/
		/* 除4095.0可以把AD值归一化，再乘上一个系数，可以调整到一个合适的范围*/
//		Outer.Kp = RP_GetValue(1) / 4095.0 * 2;				//修改Kp，调整范围：0~2
//		Outer.Ki = RP_GetValue(2) / 4095.0 * 2;				//修改Ki，调整范围：0~2
//		Outer.Kd = RP_GetValue(3) / 4095.0 * 2;				//修改Kd，调整范围：0~2
		Outer.Target = RP_GetValue(4) / 4095.0 * 816 - 408;	//修改目标值，调整范围：-408~408
		
		/*OLED显示*/
		OLED_Printf(0, 16, OLED_8X16, "Kp:%4.2f", Outer.Kp);			//显示Kp
		OLED_Printf(0, 32, OLED_8X16, "Ki:%4.2f", Outer.Ki);			//显示Ki
		OLED_Printf(0, 48, OLED_8X16, "Kd:%4.2f", Outer.Kd);			//显示Kd
		
		OLED_Printf(64, 16, OLED_8X16, "Tar:%+04.0f", Outer.Target);	//显示目标值
		OLED_Printf(64, 32, OLED_8X16, "Act:%+04.0f", Outer.Actual);	//显示实际值
		OLED_Printf(64, 48, OLED_8X16, "Out:%+04.0f", Outer.Out);		//显示输出值
		
		OLED_Update();	//OLED更新，调用显示函数后必须调用此函数更新，否则显示的内容不会更新到OLED上
		
		Serial_Printf("%f,%f,%f\r\n", Outer.Target, Outer.Actual, Outer.Out);	//串口打印目标值、实际值和输出值
																	//配合SerialPlot绘图软件，可以显示数据的波形
	}
}

void TIM1_UP_IRQHandler(void)
{
	/* 定义静态计数器（基于定时中断 tick，函数退出后保留值） */
	static uint16_t Count1, Count2;        // 分别用于内环和外环的计次分频

	if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
	{
		/* 每隔 CTRL_TICK_MS（约 1ms） 到达 */

		Key_Tick();            // 按键周期处理（非阻塞）

		/* ========== 内环（速度环） ========== */
		/* 内环通常比外环频率高，负责把速度误差快速消除 */
		Count1++;
		if (Count1 >= (INNER_PERIOD_MS / CTRL_TICK_MS))
		{
			Count1 = 0;

			/* 读取编码器增量并累加位置
			   - Encoder_Get() 返回自上次读取后的脉冲增量（与速度成正比）
			   - 若需转换到物理速度：Speed = Encoder_Get() / PULSES_PER_REV / dt
				 例如：/408.0 / 0.04（本例 40ms 周期） */
			Speed = Encoder_Get();
			Location += Speed;    // 位置为速度的积分（脉冲计数）

			/* 内环 PID 计算（速度环） */
			Inner.Actual = Speed;
			PID_Update(&Inner);

			/* 将内环输出直接用于驱动（例如 PWM） */
			Motor_SetPWM(Inner.Out);
		}

		/* ========== 外环（位置环） ========== */
		/* 外环频率较低，输出为速度参考，作为内环的 setpoint（级联） */
		Count2++;
		if (Count2 >= (OUTER_PERIOD_MS / CTRL_TICK_MS))
		{
			Count2 = 0;

			/* 位置环使用累加得到的 Location 作为被控量 */
			Outer.Actual = Location;
			PID_Update(&Outer);

			/* 把外环输出作为内环目标，构成串级控制 */
			Inner.Target = Outer.Out;
		}

		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	}
}
