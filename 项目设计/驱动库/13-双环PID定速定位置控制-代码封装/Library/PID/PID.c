/**
  *              _                    _
  *       _____ (_)  _   __  ____ _ (_)
  *      /_   / | | | | / / / __  / | |  
  *       /  /_ | | | |/ / / /_/ /_ | | 
  *      /____/ |_| |___/  \______/ |_|
  *  
  * PID 控制库
  * 使用步骤：
  *   1. 定义 PID_t 变量
  *   2. 设置增益 Kp / Ki / Kd 和输出限幅 OutMax / OutMin（使用 PID_SetXxx 进行设置）
  *   3. 周期性调用 PID_Update，库内部完成误差、积分与限幅的计算
  * 内部状态请通过 PID_Init / PID_Reset 操作，不要直接修改
  */

#include "PID.h"

/**
  * 函    数：PID 初始化
  * 参    数：PID_t * 指定结构体的地址
  * 返 回 值：无
  * 说    明：将内部状态清零；增益与限幅保持调用方设置的值
  */
void PID_Init(PID_t *p)
{
	p->Target	= 0;
	p->Actual	= 0;
	p->Out		= 0;
	p->Error0	= 0;
	p->Error1	= 0;
	p->ErrorInt	= 0;
}

/**
  * 函    数：PID 复位
  * 参    数：PID_t * 指定结构体的地址
  * 返 回 值：无
  * 说    明：清除误差与积分历史，目标值与增益保持不变
  */
void PID_Reset(PID_t *p)
{
	p->Error0	= 0;
	p->Error1	= 0;
	p->ErrorInt	= 0;
}

/**
  * 函    数：设置目标值
  * 参    数：PID_t * 指定结构体的地址，PID_F 目标值
  * 返 回 值：无
  */
void PID_SetTarget(PID_t *p, PID_F Target)
{
	p->Target = Target;
}

/**
  * 函    数：设置比例系数
  */
void PID_SetKp(PID_t *p, PID_F Kp)
{
	p->Kp = Kp;
}

/**
  * 函    数：设置积分系数
  */
void PID_SetKi(PID_t *p, PID_F Ki)
{
	p->Ki = Ki;
}

/**
  * 函    数：设置微分系数
  */
void PID_SetKd(PID_t *p, PID_F Kd)
{
	p->Kd = Kd;
}

/**
  * 函    数：PID计算及结构体变量值更新
  * 参    数：PID_t * 指定结构体的地址
  * 返 回 值：无
  * 说    明：位置式PID，调用前应设置好 Target / Actual / Kp / Ki / Kd / OutMax / OutMin
  */
void PID_Update(PID_t *p)
{
	/*获取本次误差和上次误差*/
	p->Error1 = p->Error0;					//获取上次误差
	p->Error0 = p->Target - p->Actual;		//获取本次误差，目标值减实际值，即为误差值

	/*外环误差积分（累加）*/
	/*如果Ki不为0，才进行误差积分，这样做的目的是便于调试*/
	/*因为在调试时，我们可能先把Ki设置为0，这时积分项无作用，误差消除不了，误差积分会积累到很大的值*/
	/*后续一旦Ki不为0，那么因为误差积分已经积累到很大的值了，这就导致积分项疯狂输出，不利于调试*/
	if (p->Ki != 0)							//如果Ki不为0
	{
		p->ErrorInt += p->Error0;			//进行误差积分

		/*积分限幅，防止积分饱和（积分项 Ki*ErrorInt 不超出输出范围）*/
		if (p->Ki > 0)						//如果Ki为正，按正常方向限幅
		{
			if (p->ErrorInt >  p->OutMax / p->Ki)	p->ErrorInt = p->OutMax / p->Ki;
			if (p->ErrorInt <  p->OutMin / p->Ki)	p->ErrorInt = p->OutMin / p->Ki;
		}
		else								//否则，即Ki为负，限幅方向取反
		{
			if (p->ErrorInt >  p->OutMin / p->Ki)	p->ErrorInt = p->OutMin / p->Ki;
			if (p->ErrorInt <  p->OutMax / p->Ki)	p->ErrorInt = p->OutMax / p->Ki;
		}
	}
	else									//否则
	{
		p->ErrorInt = 0;					//误差积分直接归0
	}

	/*PID计算*/
	/*使用位置式PID公式，计算得到输出值*/
	p->Out = p->Kp * p->Error0
		   + p->Ki * p->ErrorInt
		   + p->Kd * (p->Error0 - p->Error1);

	/*输出限幅*/
	if (p->Out > p->OutMax)	{p->Out = p->OutMax;}	//限制输出值最大为结构体指定的OutMax
	if (p->Out < p->OutMin)	{p->Out = p->OutMin;}	//限制输出值最小为结构体指定的OutMin
}
