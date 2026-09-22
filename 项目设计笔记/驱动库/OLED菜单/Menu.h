/**
 * @file    Menu.h
 * @brief   OLED 菜单库 — 头文件
 * @note    依赖 OLED 库与按键库。移植时只需确认 Key_GetNum() 可用，
 *          并把 MENU_KEY_* 宏改成你实际的按键编号即可。
 */
#ifndef MENU_H
#define MENU_H

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/* 标准类型定义（uint8_t 等），根据实际单片机型号包含对应头文件 */
#include "stm32f10x.h"

/* 依赖的外部库 */
#include "OLED.h"
#include "Key.h"

/*--------------------------------------------------------------------
 * 按键映射
 *
 * 按键库 Key_GetNum() 返回按键编号，此处把编号映射成菜单的四种操作：
 *   1 = 向上  2 = 向下  3 = 确认  4 = 返回
 * 如果你的按键编号不同，只需修改下面四个宏，不必改动 Menu.c。
 * 要求 Key_GetNum() 为边沿触发：没有新按键时返回 0。
 *------------------------------------------------------------------*/
#define MENU_KEY_UP       1   /* 向上移动光标 */
#define MENU_KEY_DOWN     2   /* 向下移动光标 */
#define MENU_KEY_ENTER    3   /* 确认 / 进入子菜单 */
#define MENU_KEY_BACK     4   /* 返回上一级 */

/*--------------------------------------------------------------------
 * 菜单选项结构体
 *------------------------------------------------------------------*/
typedef struct
{
    char *String;           /* 选项显示的文本，通常为字符串字面量 */
    void (*func)(void);     /* 按确认键时执行的函数；NULL 表示“返回 / 退出” */
    void *Variable;         /* 预留字段，用于关联数据；不使用填 NULL */
} Option_Class;

/*--------------------------------------------------------------------
 * 菜单全局属性结构体
 *------------------------------------------------------------------*/
typedef struct
{
    uint8_t Font_Width;     /* 字体宽度(像素)：8 或 6 */
    uint8_t Font_Height;    /* 字体高度(像素) */
    uint8_t Line_Height;    /* 行高(像素)，即每行选项的显示高度 */
    uint8_t Window_W;       /* 显示区域宽度(像素) */
    uint8_t Window_H;       /* 显示区域高度(像素)，用于计算一屏显示的行数 */
    uint8_t Menu_FontSize;  /* 菜单文字字号，对应 OLED_Printf 的 FontSize 参数 */
} MenuProperty;

extern MenuProperty Menu_Global;

/*--------------------------------------------------------------------
 * 函数接口
 *------------------------------------------------------------------*/

/**
 * @brief   读取一次按键状态并保存（供事件函数使用）
 */
void Menu_GetKey(void);

/**
 * @brief   滚动事件，返回滚动增量：
 *          +1 = 向下，-1 = 向上，0 = 无滚动
 */
int8_t Menu_RollEvent(void);

/**
 * @brief   确认事件：按下确认键返回 1，否则返回 0
 */
int8_t Menu_EnterEvent(void);

/**
 * @brief   返回事件：按下返回键返回 1，否则返回 0
 */
int8_t Menu_BackEvent(void);

/**
 * @brief   运行一个菜单（死循环，直到“返回/退出”）
 * @param   list      选项数组，每个元素描述一个菜单项
 * @param   itemCount 选项个数（数组元素个数，不是最大下标）
 * @note    退出条件：选中项 func 为 NULL 时按确认键，或按下返回键。
 *          选中项的 func 会被阻塞式调用，子菜单/功能返回后继续本菜单循环。
 */
void Menu_RunMenu(Option_Class *list, uint16_t itemCount);

#endif /* MENU_H */
