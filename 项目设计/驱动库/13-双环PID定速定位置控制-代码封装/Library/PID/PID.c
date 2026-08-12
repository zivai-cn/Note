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
	/* 积分更新与抗积分饱和（anti-windup）策略
	   - 首先计算积分候选值并对积分项进行限幅（防止积分项本身超过输出范围）
	   - 然后计算输出并对输出限幅；若输出被限幅且最近一次积分使情况更糟（朝饱和方向积累），
		 撤销最近一次积分（条件积分），避免积分继续推动输出饱和。这是一种简单且有效的 anti-windup 策略。
	*/
	if (p->Ki != 0)
	{
		/* 保存上一次积分值，用于在需要时撤销 */
		PID_F prevErrorInt = p->ErrorInt;

		/* 计算积分候选值（未限幅） */
		PID_F candErrorInt = p->ErrorInt + p->Error0;

		/* 对积分项进行限幅，使 Ki*candErrorInt 不超出输出范围 */
		if (p->Ki > 0)
		{
			if (candErrorInt >  p->OutMax / p->Ki)    candErrorInt = p->OutMax / p->Ki;
			if (candErrorInt <  p->OutMin / p->Ki)    candErrorInt = p->OutMin / p->Ki;
		}
		else
		{
			if (candErrorInt >  p->OutMin / p->Ki)    candErrorInt = p->OutMin / p->Ki;
			if (candErrorInt <  p->OutMax / p->Ki)    candErrorInt = p->OutMax / p->Ki;
		}

		p->ErrorInt = candErrorInt; /* 先暂时接受候选积分值 */

		/* 计算 PID 输出（位置式），随后根据输出限幅判断是否需要撤销积分 */
		p->Out = p->Kp * p->Error0
			   + p->Ki * p->ErrorInt
			   + p->Kd * (p->Error0 - p->Error1);

		/* 如果输出超过限幅并且最近一次积分朝着饱和方向累积，则撤销该次积分 */
		if (p->Out > p->OutMax)
		{
			/* 若积分项的本次变化方向会使输出朝上溢出（p->Error0*p->Ki>0），撤销 */
			if ((p->Error0 * p->Ki) > 0)
			{
				p->ErrorInt = prevErrorInt; /* 撤销最近一次积分 */
				/* 重新计算输出（使用撤销后的积分值） */
				p->Out = p->Kp * p->Error0
					   + p->Ki * p->ErrorInt
					   + p->Kd * (p->Error0 - p->Error1);
			}
			p->Out = p->OutMax;
		}
		else if (p->Out < p->OutMin)
		{
			/* 若积分项的本次变化方向会使输出朝下溢出（p->Error0*p->Ki<0），撤销 */
			if ((p->Error0 * p->Ki) < 0)
			{
				p->ErrorInt = prevErrorInt; /* 撤销最近一次积分 */
				/* 重新计算输出（使用撤销后的积分值） */
				p->Out = p->Kp * p->Error0
					   + p->Ki * p->ErrorInt
					   + p->Kd * (p->Error0 - p->Error1);
			}
			p->Out = p->OutMin;
		}
	}
	else
	{
		/* Ki == 0 时不进行积分 */
		p->ErrorInt = 0;
		p->Out = p->Kp * p->Error0
			   + p->Ki * p->ErrorInt
			   + p->Kd * (p->Error0 - p->Error1);

		if (p->Out > p->OutMax)    {p->Out = p->OutMax;}
		if (p->Out < p->OutMin)    {p->Out = p->OutMin;}
	}
