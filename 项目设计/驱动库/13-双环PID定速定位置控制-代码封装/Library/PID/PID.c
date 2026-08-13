/**
  *              _                   _
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
  * 功能开关（1 启用，0 禁用）
  * - PID_USE_INT_LIMIT        积分限幅：限幅 ErrorInt，使 Ki*ErrorInt 不超出输出范围（限幅值由 OutMax/OutMin 自动推导）
  * - PID_USE_INT_SEPARATION   积分分离：|误差|>SepBand 时停止积分（冻结积分项，不清零）
  * - PID_USE_VAR_INTEGRAL     变速积分：按误差大小线性缩放有效积分系数（误差过大时冻结积分项）
  * - PID_USE_DERIVATIVE_FIRST 微分先行：对实际值微分，目标值突变无冲击
  * - PID_USE_INCOMPLETE_DERIV 不完全微分：对微分项做一阶低通滤波
  * - PID_USE_OUTPUT_OFFSET    输出偏移：在输出上叠加固定偏移
  * - PID_USE_DEADBAND         输入死区：|误差|<=Deadband 时误差视为 0
  *
  * 使用方法：把对应功能改为 1，并在初始化列表或代码中设置其参数字段。
  * 参数为 0 时功能自动无效（安全默认），此时控制效果与基础版一致。
  */
#ifndef PID_USE_INT_LIMIT
#define PID_USE_INT_LIMIT        1
#endif
#ifndef PID_USE_INT_SEPARATION
#define PID_USE_INT_SEPARATION   0
#endif
#ifndef PID_USE_VAR_INTEGRAL
#define PID_USE_VAR_INTEGRAL     0
#endif
#ifndef PID_USE_DERIVATIVE_FIRST
#define PID_USE_DERIVATIVE_FIRST 0
#endif
#ifndef PID_USE_INCOMPLETE_DERIV
#define PID_USE_INCOMPLETE_DERIV 0
#endif
#ifndef PID_USE_OUTPUT_OFFSET
#define PID_USE_OUTPUT_OFFSET    0
#endif
#ifndef PID_USE_DEADBAND
#define PID_USE_DEADBAND         0
#endif

/* 浮点绝对值，不依赖 math.h */
#ifndef PID_ABS
#define PID_ABS(x) (((x) > 0) ? (x) : -(x))
#endif

/**
  * 函    数：PID 初始化
  * 参    数：PID_t * 指定结构体的地址
  * 返 回 值：无
  * 说    明：将内部状态清零；增益与限幅保持调用方设置的值
  */
void PID_Init(PID_t *p)
{
	p->Target	    = 0;
	p->Actual	    = 0;
	p->Actual1	    = 0;
	p->Out		    = 0;
	p->Error0	    = 0;
	p->Error1	    = 0;
	p->ErrorInt	    = 0;
	p->DerivFilt	= 0;
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
	p->Actual1	= 0;
	p->DerivFilt	= 0;
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
  * 函    数：设置积分分离阈值
  * 参    数：PID_t * 指定结构体的地址，PID_F 积分分离阈值
  * 返 回 值：无
  */
void PID_SetSepBand(PID_t *p, PID_F SepBand)
{
	p->SepBand = SepBand;
}

/**
  * 函    数：设置变速积分范围
  * 参    数：PID_t * 指定结构体的地址，PID_F 起始误差，PID_F 截止误差
  * 返 回 值：无
  */
void PID_SetVarIntegral(PID_t *p, PID_F Start, PID_F End)
{
	p->VarIntStart	= Start;
	p->VarIntEnd	= End;
}

/**
  * 函    数：设置不完全微分低通系数
  * 参    数：PID_t * 指定结构体的地址，PID_F 滤波系数（0~1）
  * 返 回 值：无
  */
void PID_SetDerivAlpha(PID_t *p, PID_F Alpha)
{
	p->DerivAlpha = Alpha;
}

/**
  * 函    数：设置输出偏移
  * 参    数：PID_t * 指定结构体的地址，PID_F 输出偏移值
  * 返 回 值：无
  */
void PID_SetOutOffset(PID_t *p, PID_F Offset)
{
	p->OutOffset = Offset;
}

/**
  * 函    数：设置输入死区
  * 参    数：PID_t * 指定结构体的地址，PID_F 死区范围
  * 返 回 值：无
  */
void PID_SetDeadband(PID_t *p, PID_F Deadband)
{
	p->Deadband = Deadband;
}

/**
  * 函    数：PID计算及结构体变量值更新
  * 参    数：PID_t * 指定结构体的地址
  * 返 回 值：无
  * 说    明：位置式PID，调用前应设置好 Target / Actual / Kp / Ki / Kd / OutMax / OutMin
  *          增强功能由 PID_USE_* 开关控制，对应参数字段为 0 时功能无效
  */
void PID_Update(PID_t *p)
{
	/*1.获取本次误差和上次误差*/
	p->Error1 = p->Error0;					//获取上次误差
	p->Error0 = p->Target - p->Actual;		//获取本次误差，目标值减实际值，即为误差值

	/*2.输入死区：误差小于死区视为0，避免输出抖动；同时清零上次误差，避免退出死区时微分冲击*/
#if (PID_USE_DEADBAND)
	if ((p->Deadband > 0) && (PID_ABS(p->Error0) <= p->Deadband))
	{
		p->Error0 = 0;
		p->Error1 = 0;
#if (PID_USE_DERIVATIVE_FIRST)
		p->Actual1 = p->Actual;			//同步实际值，使微分先行项归零
#endif
	}
#endif

	/*3.计算有效积分系数（积分分离 / 变速积分）*/
	PID_F kiEff = p->Ki;					//有效积分系数，默认等于Ki

#if (PID_USE_INT_SEPARATION)
	/*积分分离：误差大于SepBand时停止积分，防止大误差下积分饱和*/
	if ((p->SepBand > 0) && (PID_ABS(p->Error0) > p->SepBand))
	{
		kiEff = 0;							//停止积分
	}
#endif

#if (PID_USE_VAR_INTEGRAL)
	/*变速积分：误差在[VarIntStart, VarIntEnd]内时线性缩放积分系数，超出则停止*/
	if ((kiEff != 0) && (p->VarIntEnd > p->VarIntStart))
	{
		if (PID_ABS(p->Error0) >= p->VarIntEnd)				//误差太大，停止积分
		{
			kiEff = 0;
		}
		else if (PID_ABS(p->Error0) > p->VarIntStart)		//误差在中间区，线性缩放
		{
			kiEff = p->Ki * (p->VarIntEnd - PID_ABS(p->Error0))
					/ (p->VarIntEnd - p->VarIntStart);
		}
	}
#endif

	/*4.误差积分（累加）*/
	/*如果有效积分系数不为0，才进行误差积分，这样做的目的是便于调试*/
	/*因为在调试时，我们可能先把Ki设置为0，这时积分项无作用，误差消除不了，误差积分会积累到很大的值*/
	/*后续一旦Ki不为0，那么因为误差积分已经积累到很大的值了，这就导致积分项疯狂输出，不利于调试*/
	/*因此：Ki==0 时积分清零；kiEff==0 但 Ki!=0（积分分离/变速积分生效）时冻结积分，不累加也不清零*/
	if (kiEff != 0)							//如果有效积分系数不为0
	{
		p->ErrorInt += p->Error0;			//进行误差积分

#if (PID_USE_INT_LIMIT)
		/*积分限幅：限制ErrorInt，使 Ki*ErrorInt 不超出输出范围，防止积分饱和*/
		/*限幅值由OutMax/OutMin和Ki自动推导，无需额外设置参数*/
		if (p->Ki > 0)						//如果Ki为正，按正常方向限幅
		{
			if (p->ErrorInt >  p->OutMax / p->Ki)	p->ErrorInt = p->OutMax / p->Ki;
			if (p->ErrorInt <  p->OutMin / p->Ki)	p->ErrorInt = p->OutMin / p->Ki;
		}
		else if (p->Ki < 0)					//否则，即Ki为负，限幅方向取反
		{
			if (p->ErrorInt >  p->OutMin / p->Ki)	p->ErrorInt = p->OutMin / p->Ki;
			if (p->ErrorInt <  p->OutMax / p->Ki)	p->ErrorInt = p->OutMax / p->Ki;
		}
#endif
	}
	else if (p->Ki == 0)					//否则如果Ki为0：积分系数无效，清零积分（便于调试）
	{
		p->ErrorInt = 0;					//误差积分清零
	}
	/*否则（kiEff==0 但 Ki!=0，即积分分离/变速积分生效）：冻结积分，ErrorInt保持不变*/

	/*5.微分项计算（微分先行 / 不完全微分）*/
#if (PID_USE_DERIVATIVE_FIRST)
	/*微分先行：对实际值微分，目标值突变时不会产生微分冲击*/
	PID_F deriv = p->Actual - p->Actual1;	//实际值差分
#else
	PID_F deriv = p->Error0 - p->Error1;	//误差差分
#endif

#if (PID_USE_INCOMPLETE_DERIV)
	/*不完全微分：对微分项做一阶低通滤波，抑制高频噪声*/
	if (p->DerivAlpha > 0)
	{
		p->DerivFilt += p->DerivAlpha * (deriv - p->DerivFilt);	//滤波
		deriv = p->DerivFilt;				//用滤波后的微分值
	}
#endif

	PID_F dTerm = p->Kd * deriv;			//微分项

	/*6.位置式PID计算，得到输出值*/
	p->Out = p->Kp * p->Error0
		   + kiEff * p->ErrorInt
		   + dTerm;

#if (PID_USE_OUTPUT_OFFSET)
	/*7.输出偏移：在输出上叠加固定偏移（前馈/偏置）*/
	p->Out += p->OutOffset;
#endif

	/*8.输出限幅*/
	if (p->Out > p->OutMax)	{p->Out = p->OutMax;}	//限制输出值最大为结构体指定的OutMax
	if (p->Out < p->OutMin)	{p->Out = p->OutMin;}	//限制输出值最小为结构体指定的OutMin

#if (PID_USE_DERIVATIVE_FIRST)
	/*9.保存本次实际值，供下次微分先行使用*/
	p->Actual1 = p->Actual;
#endif
}
