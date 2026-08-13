#ifndef PID_H
#define PID_H

/* 数据类型宏：默认单精度浮点，可按需整体改为 int32_t / double 等 */
#ifndef PID_F
#define PID_F float
#endif

typedef struct {
	/* 控制量 */
	PID_F Target;			/* 目标值 */
	PID_F Actual;			/* 实际值（反馈值） */
	PID_F Out;				/* 输出值 */

	/* PID 系数 */
	PID_F Kp;				/* 比例系数 */
	PID_F Ki;				/* 积分系数 */
	PID_F Kd;				/* 微分系数 */

	/* 内部状态（由库管理） */
	PID_F Error0;			/* 本次误差 */
	PID_F Error1;			/* 上次误差 */
	PID_F ErrorInt;			/* 误差积分（自动限幅，防积分饱和） */

	/* 输出限幅 */
	PID_F OutMax;			/* 输出上限 */
	PID_F OutMin;			/* 输出下限 */

	/* 内部状态（由库管理） */
	PID_F Actual1;			/* 上次实际值（微分先行用） */
	PID_F DerivFilt;		/* 不完全微分滤波状态 */

	/* 功能参数（对应 PID_USE_* 功能开关，参数为 0 时功能无效） */
	PID_F SepBand;			/* 积分分离阈值：|误差|>此值时停止积分 */
	PID_F VarIntStart;		/* 变速积分起始误差：|误差|<=此值时全速积分 */
	PID_F VarIntEnd;		/* 变速积分截止误差：|误差|>=此值时停止积分 */
	PID_F DerivAlpha;		/* 不完全微分低通系数（0~1，越大越平滑） */
	PID_F OutOffset;		/* 输出偏移（前馈/偏置） */
	PID_F Deadband;			/* 输入死区 */
} PID_t;

void PID_Init(PID_t *p);					/* 状态清零初始化 */
void PID_Reset(PID_t *p);					/* 清除误差与积分历史 */
void PID_Update(PID_t *p);					/* PID 计算并更新结构体 */

void PID_SetTarget(PID_t *p, PID_F Target);	/* 设置目标值 */
void PID_SetKp(PID_t *p, PID_F Kp);			/* 设置比例系数 */
void PID_SetKi(PID_t *p, PID_F Ki);			/* 设置积分系数 */
void PID_SetKd(PID_t *p, PID_F Kd);			/* 设置微分系数 */

/* 功能参数设置 */
void PID_SetSepBand(PID_t *p, PID_F SepBand);			/* 设置积分分离阈值 */
void PID_SetVarIntegral(PID_t *p, PID_F Start, PID_F End);	/* 设置变速积分范围 */
void PID_SetDerivAlpha(PID_t *p, PID_F Alpha);			/* 设置不完全微分低通系数 */
void PID_SetOutOffset(PID_t *p, PID_F Offset);			/* 设置输出偏移 */
void PID_SetDeadband(PID_t *p, PID_F Deadband);			/* 设置输入死区 */

#endif /* PID_H */
