#include "menu.h"
#include "OLED.h"

#include "MyRTC.h"
#include "Key.h"
#include "Timer.h"
#include "Serial.h"

#include "Flashlight.h"
#include "Time_execute.h"
#include "Heart_Detect.h"
#include "Bluetooth_Serial.h"

void External_Init()
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

/**********************************************************/
// 时间设置包装函数
void ChangeYear(void) {
    Change_First_Page_Time(0);
}

void ChangeMonth(void) {
    Change_First_Page_Time(1);
}

void ChangeDay(void) {
    Change_First_Page_Time(2);
}

void ChangeHour(void) {
    Change_First_Page_Time(3);
}

void ChangeMinute(void) {
    Change_First_Page_Time(4);
}

void ChangeSecond(void) {
    Change_First_Page_Time(5);
}

/**********************************************************/
// 时间设置菜单
void Menu_RunTimeSetMenu(void)
{
	static struct Option_Class Menu_TimeSetOptionList[] = {
		{"<<<", NULL, NULL, 0},
		{"Year", ChangeYear, NULL, 0},
		{"Month", ChangeMonth, NULL, 0},
		{"Day", ChangeDay, NULL, 0},
		{"Hour", ChangeHour, NULL, 0},
		{"Minute", ChangeMinute, NULL, 0},
		{"Second", ChangeSecond, NULL, 0}};
	Menu_RunMenu(Menu_TimeSetOptionList,6);
}

// 第三级菜单：多功能
void Menu_RunToolsMenu(void)
{
	static struct Option_Class Menu_ToolsOptionList[] = {
		{"<<<", NULL, NULL, 0},
		{"Flashlight", Flashlight, NULL, 0},
		{"Blood_Oxygen", SPO2_function, NULL, 0},
		{"Heart_Rate", Heart_function, NULL, 0}};
	Menu_RunMenu(Menu_ToolsOptionList,3);
}
// 第二级菜单：时间设置、功能
void Menu_RunSecondMenu(void)
{
	static struct Option_Class Menu_SecondOptionList[] = {
		{"<<<", NULL, NULL, 0},
		{"Time Setting", Menu_RunTimeSetMenu, NULL, 0},
		{"Tools", Menu_RunToolsMenu, NULL, 0}};
	Menu_RunMenu(Menu_SecondOptionList,2);
}
// 第一级菜单：显示时间
void Menu_RunMainMenu(void)
{
	while (1)
	{
		Menu_GetKey(); // 获取按键值
		// 显示时间
		MyRTC_ReadTime();

		OLED_Clear();
		OLED_Printf(0,0,OLED_6X8,"%d-%d-%d",MyRTC_Time[0],MyRTC_Time[1],MyRTC_Time[2]);
		OLED_Printf(31,18,OLED_8X16,"%02d:%02d:%02d",MyRTC_Time[3],MyRTC_Time[4],MyRTC_Time[5]);
		OLED_Update();
		
		if (Menu_EnterEvent()) // 按键3进入第二级菜单
		{
			Menu_RunSecondMenu();
		}
	}
}




/**********************************************************/