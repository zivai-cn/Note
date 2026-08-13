/**
 * @file    Menu_Data.c
 * @brief   菜单使用示例 — 三级菜单：主界面 → 二级菜单 → 三级菜单
 * @note    与 Menu.h / Menu.c 配套使用
 */
#include "Menu_Data.h"
#include "Menu.h"

#include "OLED.h"
#include "Key.h"
#include "MyRTC.h"
#include "Timer.h"
#include "Serial.h"
#include "Flashlight.h"
#include "Time_execute.h"
#include "Heart_Detect.h"

/*--------------------------------------------------------------------
 * 外设初始化（在 main 中调用一次）
 *------------------------------------------------------------------*/
void External_Init(void)
{
    Flashlight_LED_Init();
    OLED_Init();
    MyRTC_Init();
    MyRTC_SetTime();
    Key_Init();
    Timer_Init();
    MAX30102_Init();
    Serial_Init(USART1, 115200);
}

/*--------------------------------------------------------------------
 * 时间设置的包装函数（进入后按左右键等修改对应位）
 *------------------------------------------------------------------*/
static void ChangeYear(void)   { Change_First_Page_Time(0); }
static void ChangeMonth(void)  { Change_First_Page_Time(1); }
static void ChangeDay(void)    { Change_First_Page_Time(2); }
static void ChangeHour(void)   { Change_First_Page_Time(3); }
static void ChangeMinute(void) { Change_First_Page_Time(4); }
static void ChangeSecond(void) { Change_First_Page_Time(5); }

/*--------------------------------------------------------------------
 * 第三级菜单：时间设置
 * 第一项 "<<<" 的 func 为 NULL，按确认键即返回上一级
 *------------------------------------------------------------------*/
static void Menu_RunTimeSetMenu(void)
{
    static Option_Class TimeSetList[] = {
        {"<<<",    NULL,        NULL},
        {"Year",   ChangeYear,  NULL},
        {"Month",  ChangeMonth, NULL},
        {"Day",    ChangeDay,   NULL},
        {"Hour",   ChangeHour,  NULL},
        {"Minute", ChangeMinute,NULL},
        {"Second", ChangeSecond,NULL}
    };
    Menu_RunMenu(TimeSetList, 7);   /* 7 个选项 */
}

/*--------------------------------------------------------------------
 * 第三级菜单：多功能
 *------------------------------------------------------------------*/
static void Menu_RunToolsMenu(void)
{
    static Option_Class ToolsList[] = {
        {"<<<",         NULL,             NULL},
        {"Flashlight",  Flashlight,       NULL},
        {"Blood_Oxygen",SPO2_function,    NULL},
        {"Heart_Rate",  Heart_function,   NULL}
    };
    Menu_RunMenu(ToolsList, 4);         /* 4 个选项 */
}

/*--------------------------------------------------------------------
 * 第二级菜单：时间设置、多功能
 *------------------------------------------------------------------*/
static void Menu_RunSecondMenu(void)
{
    static Option_Class SecondList[] = {
        {"<<<",         NULL,               NULL},
        {"Time Setting",Menu_RunTimeSetMenu,NULL},
        {"Tools",       Menu_RunToolsMenu,  NULL}
    };
    Menu_RunMenu(SecondList, 3);        /* 3 个选项 */
}

/*--------------------------------------------------------------------
 * 第一级：主界面（显示时间），按确认键进入第二级菜单
 *------------------------------------------------------------------*/
void Menu_RunMainMenu(void)
{
    while (1)
    {
        Menu_GetKey();                  /* 读取按键 */
        MyRTC_ReadTime();               /* 刷新时间 */

        OLED_Clear();
        OLED_Printf(0, 0, OLED_6X8, "%d-%d-%d", MyRTC_Time[0], MyRTC_Time[1], MyRTC_Time[2]);
        OLED_Printf(31, 18, OLED_8X16, "%02d:%02d:%02d", MyRTC_Time[3], MyRTC_Time[4], MyRTC_Time[5]);
        OLED_Update();

        if (Menu_EnterEvent())          /* 确认键进入二级菜单 */
        {
            Menu_RunSecondMenu();
        }
    }
}
