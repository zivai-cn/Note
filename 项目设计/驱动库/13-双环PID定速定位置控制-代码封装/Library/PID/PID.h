#ifndef PID_H
#define PID_H



#ifdef __cplusplus
extern "C" {
#endif

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
} PID_t;

void PID_Init(PID_t *p);					/* 状态清零初始化 */
void PID_Reset(PID_t *p);					/* 清除误差与积分历史 */
void PID_Update(PID_t *p);					/* PID 计算并更新结构体 */

void PID_SetTarget(PID_t *p, PID_F Target);	/* 设置目标值 */
void PID_SetKp(PID_t *p, PID_F Kp);			/* 设置比例系数 */
void PID_SetKi(PID_t *p, PID_F Ki);			/* 设置积分系数 */
void PID_SetKd(PID_t *p, PID_F Kd);			/* 设置微分系数 */

#ifdef __cplusplus
}
#endif

#endif /* PID_H */
