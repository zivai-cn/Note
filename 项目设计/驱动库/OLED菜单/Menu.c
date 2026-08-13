#include "Menu.h"
/*菜单全局属性*/
struct MenuProperty Menu_Global = {
	.Font_Width = 8,    // 字体宽度 8 或 6
	.Font_Height = 16,    // 字体高度
	
	.Line_Height = 16,    // 行高

	.Window_W = 128, // 窗口宽度
	.Window_H = 64,    // 窗口高度

  	.Menu_FontSize = OLED_8X16, // 菜单默认字体大小
};

//////////////////////////////////////////////////////////////////
/**
 * 函    数：菜单显示光标
 * 参    数：Target_Cur_X 光标目标X位置
 * 参    数：Target_Cur_Y 光标目标Y位置
 * 参    数：Target_Cur_W 光标目标宽度
 * 参    数：Target_Cur_H 光标目标高度
 * 返 回 值：无
 * 说    明：输入光标目标位置和尺寸, 光标会在多次打印中逐渐接近目标位置和尺寸;
 */
void Menu_ShowCursor(uint8_t Target_Cursor_X, uint8_t Target_Cursor_Y, uint8_t Target_Cursor_W, uint8_t Target_Cursor_H)
{
	OLED_ReverseArea(Target_Cursor_X, Target_Cursor_Y, Target_Cursor_W, Target_Cursor_H); // 反相光标
}

//////////////////////////////////////////////////////////////////

/*菜单用到的按键函数独立出来,方便移植和修改,比如没有编码器可以用上下两个按键代替;*/
static uint8_t Menu_KeyNum = 0; // 保存当前按键值

void Menu_GetKey(void) // 获取按键值并保存
{
	Menu_KeyNum = Key_GetNum();
}
int8_t Menu_RollEvent(void) // 菜单滚动
{
	if (Menu_KeyNum == 1) // 按键1用于向上
	{
		return 1;
	}
	if (Menu_KeyNum == 2) // 按键2用于向下
	{
		return -1;
	}
	return 0;
}
int8_t Menu_EnterEvent(void) // 菜单确认
{
	return Menu_KeyNum == 3; // 确认键接到按键3;
}
int8_t Menu_BackEvent(void) // 菜单返回
{
	return Menu_KeyNum == 4; // 返回键接到按键4;
}

/////////////////////////////////////////////////////////
/**
 * 函    数：菜单运行
 * 参    数：选项列表
 * 返 回 值：
 * 说    明：把选项列表显示出来,并根据按键事件执行相应操作
 */
void Menu_RunMenu(struct Option_Class *Option_List, uint8_t Catch_MAX )
{
  uint8_t Roll_Event = 0;        // 记录菜单滚动事件
  int8_t Catch_i = 0;            // 选中下标默认为0
  int8_t Cursor_i = 0;            // 光标下标默认为0

  uint8_t Cursor_MAX=Menu_Global.Window_H / Menu_Global.Line_Height; // 选项下标最大值(根据窗口高度和行高计算得出)

	while (1)
	{
		Menu_GetKey(); // 获取按键值
		if (Menu_EnterEvent())//如果按下确认键
		{
			/*如果功能不为空则执行功能,否则返回*/
			if (Option_List[Cursor_i].func)
			{
				Option_List[Cursor_i].func();
			}
			else
			{
				return;
			}
		}
		if (Menu_BackEvent())//如果按下返回键
		{
			return;
		}

		//根据按键事件更改选中下标和光标下标
		Roll_Event = Menu_RollEvent();
		if (Roll_Event)
		{
			/*更新下标*/
			Cursor_i += Roll_Event;
			Catch_i += Roll_Event;
			/*限制选中下标*/
			if (Catch_i > Catch_MAX)
			{
				Catch_i = Catch_MAX;
			}
			else if (Catch_i < 0)
			{
				Catch_i = 0;
			}
			/*限制光标下标*/
			if (Cursor_i >= Cursor_MAX)
			{
				Cursor_i = Cursor_MAX - 1;
			}
			else if (Cursor_i > Catch_MAX)
			{
				Cursor_i = Catch_MAX;
			}
			else if (Cursor_i < 0) 
			{
				Cursor_i = 0;
			}
		}

		/**********************************************************/

		OLED_Clear();

		// 显示起始下标（根据光标位置动态调整）
		uint8_t Show_i = 0;
		
		// 计算显示起始位置
		if (Catch_i >= Cursor_MAX)
		{
			// 如果选中项超过显示范围，从后往前显示
			Show_i = Catch_i - Cursor_MAX + 1;
		}
		else
		{
			// 否则从头开始显示
			Show_i = 0;
		}

		for (uint8_t i = 0; i < Cursor_MAX; i++) // 遍历显示选项
		{
			if (Show_i + i < 0)
			{
				continue;
			}
			if (Show_i + i > Catch_MAX)
			{
				break;
			}

			//格式化显示字符串
			OLED_Printf(
				0,Menu_Global.Line_Height * (i ),Menu_Global.Menu_FontSize,
				/*要显示的字符串*/
				Option_List[Show_i + i].String);
		}

		//显示光标
		uint8_t cursor_y_pos = (Catch_i - Show_i) * Menu_Global.Line_Height;
		uint8_t cursor_width = (uint8_t)strlen(Option_List[Catch_i].String) * Menu_Global.Font_Width;
		Menu_ShowCursor(0, cursor_y_pos, cursor_width, Menu_Global.Line_Height);
		
    OLED_Update();
	}
}