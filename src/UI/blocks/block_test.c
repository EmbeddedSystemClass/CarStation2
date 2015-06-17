/*
 * block_test.c
 *
 * 用于显示测试数据
 *
 *  Created on: Jan 27, 2015
 *      Author: daniel
 */

#include "block.h"
#include <chprintf.h>

static void test_load(const struSize* size, void* param)
{
}

static void test_unload(void)
{
}

static void test_msg(Msg* msg)
{
	char		temp[15];
	font_t		font;

	if (msg->Id == MSG_UI_LIGHT)
	{
		gdispFillArea(0, 0, 128, 9, 0);
		chsnprintf(temp, 15, "Light:%d",
					msg->Param.Light.Light);

		font = gdispOpenFont("UI2");
		gdispDrawString(0, 0, temp, font, 1);
		gdispCloseFont(font);
	}
}

const struBlockFunctions	block_test =
{
	test_load,
	test_unload,
	test_msg,
	NULL,
};

