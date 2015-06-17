/*
 * block_sh21.c
 *
 *  Created on: 2014年11月20日
 *      Author: daniel
 */

#include "block.h"
#include <chprintf.h>

static struSize		s_Pos;

static void sh21_load(const struSize* size, void* param)
{
	(void)param;

	s_Pos = *size;
}

static void sh21_unload(void)
{
}

static void sh21_msg(Msg* msg)
{
	char		temp[12];
	font_t		font;

	if (msg->Id == MSG_UI_TANDH_IN)
	{
		gdispFillArea(0, 42, 128, 10, 0);

		chsnprintf(temp, 12, "%d %d",
					msg->Param.SHT21Data.Temperature / 10,
					msg->Param.SHT21Data.Humidity / 10);

		font = gdispOpenFont("UI1");
		gdispDrawString(0, 42, temp, font, 1);
		gdispCloseFont(font);
	}
	else if (msg->Id == MSG_UI_TANDH_OUT)
	{
		gdispFillArea(0, 52, 128, 10, 0);

		chsnprintf(temp, 12, "%d %d",
					msg->Param.SHT21Data.Temperature / 10,
					msg->Param.SHT21Data.Humidity / 10);

		font = gdispOpenFont("UI1");
		gdispDrawString(0, 52, temp, font, 1);
		gdispCloseFont(font);
	}
}

const struBlockFunctions	block_sh21 =
{
	sh21_load,
	sh21_unload,
	sh21_msg,
	NULL,
};
