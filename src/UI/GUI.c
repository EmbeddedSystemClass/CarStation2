/*
 * GUI.c
 *
 *  Created on: 2014年11月18日
 *      Author: daniel
 */

#include "GUI.h"

// Mailbox定义
#define GUI_MB_SIZE			10
static void*	gui_mb_buffer[GUI_MB_SIZE];

MAILBOX_DECL(gui_mb, gui_mb_buffer, GUI_MB_SIZE);

// 函数定义

// GUI工作线程
static WORKING_AREA(guiThread, 512);
__attribute__((noreturn))
static msg_t gui_Thread(void *arg)
{
	(void)arg;
	msg_t	ret, msg;

	// 初始化gfx
	gfxInit();

	// 设置初始的对比度（亮度）
	gdispSetContrast(40);

	// 加载首页（根据配置）
	// TODO:测试，默认加载main页面
	LoadFirstPage(PAGE_MAIN);


	// 循环处理Mailbox消息
	while (true)
	{
		ret = chMBFetch(&gui_mb, &msg, MS2ST(100));

		if (ret == RDY_OK)
		{
			// 过滤出特别的消息（例如：光照指数，用于调光；开关显示屏等）
			// TODO

			// 将消息转给各个block处理
			SendMsgToPage((Msg*)msg);

			// 释放msg（msg实际是指向消息块的指针）
			MSG_FREE((void*)msg);
		}

		// 做其他检查事务
		// TODO
	}
	//chThdExit(0);
}


bool_t InitGUI(void)
{
	//创建GUI线程，由GUI线程进行初始化和后续的工作
	chThdCreateStatic(guiThread, sizeof(guiThread), NORMALPRIO, gui_Thread, NULL);

	return true;
}


void test(void)
{
	font_t		font;
	GHandle GW1, GW2, GW3;
	int			i;

	// 测试gfx
	gfxInit();
	//gdispDrawLine(0, 0, 0, 50, 1);
	//gdispFlush();
	//gdispDrawLine(50, 50, 50, 0, 1);
	//gdispFlush();

	font = gdispOpenFont("fixed_5x8");
	gdispDrawString(0, 0, "This is test.", font, 1);
	gdispCloseFont(font);
	gdispFlush();

	font = gdispOpenFont("LargeNumbers");
	gdispDrawString(0, 10, "12:30", font, 1);
	gdispCloseFont(font);
	gdispFlush();

	gdispSetPowerMode(powerOff);
	gdispSetPowerMode(powerOn);

	gdispSetContrast(50);
	gdispSetContrast(0);
	gdispSetContrast(100);
	gdispSetContrast(50);

	chThdSleepMilliseconds(3000);

	// test consle
	font = gdispOpenFont("UI2");

	{
	  GWindowInit		wi;

	  wi.show = TRUE;
	  wi.x = 0; wi.y = 0; wi.width = gdispGetWidth(); wi.height = gdispGetHeight();
	  GW1 = gwinConsoleCreate(0, &wi);
	}

	gwinSetFont(GW1, font);
	gwinSetColor(GW1, 1);
	gwinClear(GW1);
	gdispFlush();

	// write console
	i = 0;
	while (TRUE)
	{
	  gwinPrintf(GW1,"Test \033bnumber \033B%d.\r\n", i++);
	  gdispFlush();
	  gfxSleepMilliseconds(1000);
	}
}
