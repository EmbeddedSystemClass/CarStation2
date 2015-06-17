/*
 * GUI.h
 *
 *  Created on: 2014年11月18日
 *      Author: daniel
 */

#ifndef GUI_H_
#define GUI_H_

#include <FreeRTOS.h>
#include <gfx.h>

#include "Msg/Msg.h"

// 按键事件
typedef enum
{
	BUTTON_LEFT,			// 左侧按键
	BUTTON_RIGHT,			// 右侧按键
	BUTTON_LEFT_LONG,		// 左侧按键 长按
	BUTTON_RIGHT_LONG,		// 右侧按键 长按
} enumButtonPress;

typedef enum
{
	ACTION_NULL,			// 无动作（或者可以考虑将这个按键事件函数设置为NULL）
	ACTION_OK,
	ACTION_BACK,
	ACTION_MSG,				// 提供给定义页面时使用，按键消息直接传递给Page。正常页面不会返回该值
} enumAction;

typedef struct
{
	uint8_t		x;
	uint8_t		y;
	uint8_t		cx;
	uint8_t		cy;
} struSize;

// GUI block的各个入口函数
typedef void (*LoadFunc_t)(const struSize* size, void* param);	//param：用于初始化相同块的不同风格（可能有字体不同，大小不同，显示的信息页可能不同）
typedef void (*UnloadFunc_t)(void);
typedef void (*MsgFunc_t)(Msg* msg);

typedef enumAction (*ButtonFunc_t)(enumButtonPress button);

typedef struct
{
	LoadFunc_t		load;
	UnloadFunc_t	unload;
	MsgFunc_t		msg;
	ButtonFunc_t	buttonPress;
} struBlockFunctions;

typedef struct
{
	const struSize				size;
	const struBlockFunctions*	entry;
	void*						param;		// Load使用的参数，用于指定block是以什么形式显示
} struBlock;

// 页面编号
typedef enum
{
	PAGE_DEBUG,				// 输出调试信息（Console？）
	PAGE_MAIN,				// 主页1
	PAGE_MAIN2,				// 主页2
	PAGE_MAIN3,				// 主页3
	PAGE_SETTING,			// 设置页面
	PAGE_GPSINFO,			// GPS详细信息页面
	PAGE_MAXMIN,			// （温湿度）最大值页面（过去24小时，每个星期，每个月，历史）
	PAGE_HISTORY,			// 温湿度历史值（曲线？）
} enumPage;

typedef struct
{
	const int8_t		name;		// 使用enumPage枚举
	const int8_t		blockcount;
	const struBlock*	blocks;

	// 以下为按键处理方式（使用enumAction）
	const int8_t		leftbuttonaction;
	const int8_t		rightbuttonaction;
	const int8_t		leftlongaction;
	const int8_t		rightlongaction;
} struPage;

typedef union
{
	struct		struSH21Data
	{
		struSize	size;
		int			nTemperature;
		int			nHumidity;
	}	SH21Data;
} guiMem;


bool_t InitGUI(void);

void LoadFirstPage(enumPage name);
void LoadPage(const struPage* page);
void UnloadPage(const struPage* page);
void SendMsgToPage(Msg* msg);


// 发送消息到GUI Mailbox



#endif /* GUI_H_ */
