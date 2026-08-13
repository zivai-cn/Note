/**
 * @file    Menu.c
 * @brief   OLED 菜单库 — 实现
 * @note    驱动方式为“死循环 + 全屏重绘”，结构简单、占用小；
 *          按键事件独立成函数，方便移植到编码器或独立按键等不同方案。
 */
#include "Menu.h"

/*--------------------------------------------------------------------
 * 菜单全局属性默认值
 * 与 OLED 实际尺寸、字号保持一致即可
 *------------------------------------------------------------------*/
MenuProperty Menu_Global = {
    .Font_Width     = 8,        /* 字体宽度：8 或 6 */
    .Font_Height    = 16,       /* 字体高度 */
    .Line_Height    = 16,       /* 行高 */
    .Window_W       = 128,      /* 显示区域宽度 */
    .Window_H       = 64,       /* 显示区域高度 */
    .Menu_FontSize  = OLED_8X16 /* 菜单字号 */
};

/*--------------------------------------------------------------------
 * 按键事件层
 *
 * 把“按键编号”翻译成菜单的四种操作，与菜单核心逻辑解耦。
 * 修改按键映射：改 MENU_KEY_* 宏即可，或直接改写下面的函数。
 *------------------------------------------------------------------*/
static uint8_t Menu_KeyNum = 0;   /* 保存最近一次读取到的按键值 */

void Menu_GetKey(void)
{
    Menu_KeyNum = Key_GetNum();
}

/* 滚动增量：+1 向下，-1 向上，0 无操作 */
int8_t Menu_RollEvent(void)
{
    if (Menu_KeyNum == MENU_KEY_UP)
    {
        return -1;              /* 向上：选中下标减 1 */
    }
    if (Menu_KeyNum == MENU_KEY_DOWN)
    {
        return 1;               /* 向下：选中下标加 1 */
    }
    return 0;
}

/* 确认键：按下返回 1 */
int8_t Menu_EnterEvent(void)
{
    return (Menu_KeyNum == MENU_KEY_ENTER);
}

/* 返回键：按下返回 1 */
int8_t Menu_BackEvent(void)
{
    return (Menu_KeyNum == MENU_KEY_BACK);
}

/*--------------------------------------------------------------------
 * 光标显示
 *------------------------------------------------------------------*/
static void Menu_ShowCursor(uint8_t Cur_X, uint8_t Cur_Y, uint8_t Cur_W, uint8_t Cur_H)
{
    /* 反相高亮指定区域，形成光标框 */
    OLED_ReverseArea(Cur_X, Cur_Y, Cur_W, Cur_H);
}

/*--------------------------------------------------------------------
 * 菜单运行核心
 *------------------------------------------------------------------*/
void Menu_RunMenu(Option_Class *list, uint16_t itemCount)
{
    int16_t select_i = 0;              /* 当前选中项下标：0 ~ itemCount-1 */
    int8_t  roll     = 0;              /* 本次滚动增量 */
    uint8_t perPage  = Menu_Global.Window_H / Menu_Global.Line_Height; /* 一屏显示的行数 */

    if (itemCount == 0)                /* 空菜单直接退出 */
    {
        return;
    }
    if (perPage == 0)                  /* 防御：一屏至少要能显示 1 行 */
    {
        perPage = 1;
    }

    while (1)
    {
        Menu_GetKey();                 /* 读取一次按键 */

        /* 确认：执行选中项的功能；func 为 NULL 视为“返回 / 退出” */
        if (Menu_EnterEvent())
        {
            if (list[select_i].func)
            {
                list[select_i].func(); /* 子菜单 / 功能为阻塞式调用，返回后继续本菜单 */
            }
            else
            {
                return;
            }
        }

        /* 返回：退出本菜单 */
        if (Menu_BackEvent())
        {
            return;
        }

        /* 滚动：更新选中下标并做边界钳位 */
        roll = Menu_RollEvent();
        if (roll)
        {
            select_i += roll;
            if (select_i < 0)
            {
                select_i = 0;
            }
            else if (select_i >= itemCount)
            {
                select_i = itemCount - 1;
            }
        }

        /***************** 绘制一帧菜单 *****************/

        OLED_Clear();

        /* 计算本帧从哪个选项开始显示（滚动时保证选中项可见） */
        uint16_t show_i = 0;
        if (select_i >= perPage)
        {
            show_i = (uint16_t)(select_i - perPage + 1);
        }

        /* 逐行绘制选项文本 */
        for (uint8_t i = 0; i < perPage; i++)
        {
            uint16_t idx = show_i + i;
            if (idx >= itemCount)
            {
                break;                 /* 后面没有更多选项了 */
            }
            OLED_Printf(0,
                        Menu_Global.Line_Height * i,
                        Menu_Global.Menu_FontSize,
                        list[idx].String);
        }

        /* 绘制光标：宽 = 文本长度 × 字体宽度，高 = 行高 */
        uint8_t cur_y = (uint8_t)((int16_t)(select_i - show_i) * Menu_Global.Line_Height);
        uint8_t cur_w = (uint8_t)strlen(list[select_i].String) * Menu_Global.Font_Width;
        Menu_ShowCursor(0, cur_y, cur_w, Menu_Global.Line_Height);

        OLED_Update();
    }
}
