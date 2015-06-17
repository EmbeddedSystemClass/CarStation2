/*
 * block_control.c
 *
 * 用于通用的控制液晶屏亮暗、开关的行为
 *
 *  Created on: Jan 27, 2015
 *      Author: daniel
 */

#include "block.h"

static void control_load(const struSize* size, void* param)
{
}

static void control_unload(void)
{
}

static void control_msg(Msg* msg)
{
	switch (msg->Id)
	{
	case MSG_UI_ONOFF:
		gdispSetPowerMode(msg->Param.DisplayOnOff.IsOn ? powerOn : powerOff);
		break;

	case MSG_UI_CONTRAST:
		gdispSetContrast(msg->Param.DisplayContrast.Contrast);
		break;

	default:
		break;
	}
}

const struBlockFunctions	block_control =
{
	control_load,
	control_unload,
	control_msg,
	NULL,
};
