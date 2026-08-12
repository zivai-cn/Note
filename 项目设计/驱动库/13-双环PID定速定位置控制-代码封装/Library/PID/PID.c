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

/* 选择积分抗饱和（anti-windup）方案：
	- AW_NONE:        不使用积分（Ki 被视为 0）
	- AW_CLAMP:       经典积分限幅（限制 ErrorInt 使 Ki*ErrorInt 不超出输出范围）
	- AW_CONDITIONAL: 条件积分（当输出被饱和且本次积分朝饱和方向累积时，撤销本次积分）
	- AW_BACK_CALC:   反向校正（back-calculation），需要调整 AW_GAIN

	使用方法：编辑此文件把下面的 AW_METHOD 宏改为需要的方案，或在编译器命令中定义。
*/
#define AW_NONE        0
#define AW_CLAMP       1
#define AW_CONDITIONAL 2
#define AW_BACK_CALC   3

/* 默认使用条件积分（稳妥的 anti-windup） */
#ifndef AW_METHOD
#define AW_METHOD AW_CONDITIONAL
#endif

/* 仅在 AW_BACK_CALC 时有效：离散反向校正增益（越大收敛越快，但可能引入振荡） */
#ifndef AW_GAIN
#define AW_GAIN 0.05f
#endif

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
	/* 积分与抗饱和策略统一在下面通过 AW_METHOD 选择实现 */
	
	/* 导数项（差分） */
	PID_F dTerm = p->Kd * (p->Error0 - p->Error1);

#if (AW_METHOD == AW_NONE)
	/* 不使用积分 */
	p->ErrorInt = 0;
	p->Out = p->Kp * p->Error0 + p->Ki * p->ErrorInt + dTerm;

#elif (AW_METHOD == AW_CLAMP)
	/* 经典积分限幅：先积分再对 ErrorInt 进行限幅，保证 Ki*ErrorInt 不超出输出范围 */
	p->ErrorInt += p->Error0;
	if (p->Ki > 0)
	{
		if (p->ErrorInt >  p->OutMax / p->Ki)    p->ErrorInt = p->OutMax / p->Ki;
		if (p->ErrorInt <  p->OutMin / p->Ki)    p->ErrorInt = p->OutMin / p->Ki;
	}
	else
	{
		if (p->ErrorInt >  p->OutMin / p->Ki)    p->ErrorInt = p->OutMin / p->Ki;
		if (p->ErrorInt <  p->OutMax / p->Ki)    p->ErrorInt = p->OutMax / p->Ki;
	}
	p->Out = p->Kp * p->Error0 + p->Ki * p->ErrorInt + dTerm;
	if (p->Out > p->OutMax)    {p->Out = p->OutMax;}
	if (p->Out < p->OutMin)    {p->Out = p->OutMin;}

#elif (AW_METHOD == AW_CONDITIONAL)
	/* 条件积分：当输出因为积分而导致朝饱和方向累积时，撤销最近一次积分 */
	{
		PID_F prevErrorInt = p->ErrorInt;
		PID_F candErrorInt = p->ErrorInt + p->Error0;

		/* 对积分候选做与限幅相容的裁剪，避免积分项太大 */
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

		p->ErrorInt = candErrorInt;
		p->Out = p->Kp * p->Error0 + p->Ki * p->ErrorInt + dTerm;

		if (p->Out > p->OutMax)
		{
			if ((p->Error0 * p->Ki) > 0)
			{
				p->ErrorInt = prevErrorInt; /* 撤销积分 */
				p->Out = p->Kp * p->Error0 + p->Ki * p->ErrorInt + dTerm;
			}
			p->Out = p->OutMax;
		}
		else if (p->Out < p->OutMin)
		{
			if ((p->Error0 * p->Ki) < 0)
			{
				p->ErrorInt = prevErrorInt; /* 撤销积分 */
				p->Out = p->Kp * p->Error0 + p->Ki * p->ErrorInt + dTerm;
			}
			p->Out = p->OutMin;
		}
	}

#elif (AW_METHOD == AW_BACK_CALC)
	/* 反向校正（back-calculation）离散实现：
	   - 先计算积分候选与无饱和输出，然后用饱和误差修正积分：
		 ErrorInt += Error0 + AW_GAIN * (u_sat - u_unsat)
	   - AW_GAIN 需根据采样周期与回缴时间常数调整，默认为文件顶部的 AW_GAIN。
	*/
	{
		PID_F candErrorInt = p->ErrorInt + p->Error0;
		PID_F u_unsat = p->Kp * p->Error0 + p->Ki * candErrorInt + dTerm;
		PID_F u_sat = u_unsat;
		if (u_sat > p->OutMax) u_sat = p->OutMax;
		if (u_sat < p->OutMin) u_sat = p->OutMin;

		/* 用饱和误差校正积分（离散近似） */
		p->ErrorInt = p->ErrorInt + p->Error0 + AW_GAIN * (u_sat - u_unsat);

		/* 为安全起见，对积分再做限幅（避免极端值） */
		if (p->Ki > 0)
		{
			if (p->ErrorInt >  p->OutMax / p->Ki)    p->ErrorInt = p->OutMax / p->Ki;
			if (p->ErrorInt <  p->OutMin / p->Ki)    p->ErrorInt = p->OutMin / p->Ki;
		}
		else if (p->Ki < 0)
		{
			if (p->ErrorInt >  p->OutMin / p->Ki)    p->ErrorInt = p->OutMin / p->Ki;
			if (p->ErrorInt <  p->OutMax / p->Ki)    p->ErrorInt = p->OutMax / p->Ki;
		}

		p->Out = p->Kp * p->Error0 + p->Ki * p->ErrorInt + dTerm;
		if (p->Out > p->OutMax)    {p->Out = p->OutMax;}
		if (p->Out < p->OutMin)    {p->Out = p->OutMin;}
	}

#else
#error "Unknown AW_METHOD"
#endif
