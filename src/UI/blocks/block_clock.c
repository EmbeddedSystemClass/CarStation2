/*
 * block_clock.c
 *
 *  Created on: 2014年11月20日
 *      Author: daniel
 */

#include "block.h"

static struSize		s_Pos;

static void clock_load(const struSize* size, void* param)
{
	(void)param;
	// TODO:测试代码
	//gdispSetClip(size->x, size->y, size->cx, size->cy);

	// 保持位置和大小
	s_Pos = *size;
}

static void clock_unload(void)
{
}

static void clock_msg(Msg* msg)
{
	if (msg->Id == MSG_UI_CLOCK)
	{
		font_t		font;
		char		ctime[6];

		// Clear
		gdispFillArea(0, 10, 128, 22, 0);

		snprintf(ctime, 6, "%02d:%02d", msg->Param.UIClock.Hour, msg->Param.UIClock.Minute);
		font = gdispOpenFont("LargeNumbers");
		gdispDrawString(0, 10, ctime, font, 1);
		gdispCloseFont(font);
	}
}


const struBlockFunctions	block_clock =
{
	clock_load,
	clock_unload,
	clock_msg,
	NULL,
};

