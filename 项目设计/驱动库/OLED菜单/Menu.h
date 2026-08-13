#ifndef __MENU_H
#define __MENU_H

#include "stm32f10x.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

//////////////////////////////
//引用外部库文件
#include "OLED.h"
#include "Key.h"
//////////////////////////////

struct Option_Class 
{ 
    char *String;        // 选项显示的文本字符串指针
    void (*func)(void);  // 点击选项时执行的函数指针
    float *Variable;     // 指向要显示或修改的变量（浮点型）
    uint8_t StrLen;      // 字符串长度
};

struct MenuProperty
{	
	int8_t Font_Width;
	int8_t Font_Height;

	int8_t Line_Height;
	
	uint8_t Window_W;
	uint8_t Window_H;

  	uint8_t Menu_FontSize;
};
extern struct MenuProperty Menu_Global;

void Menu_RunMenu(struct Option_Class *Option_List, uint8_t Catch_MAX );

#endif
