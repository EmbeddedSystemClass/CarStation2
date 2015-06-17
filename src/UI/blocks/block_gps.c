/*
 * block_gps.c
 *
 *  Created on: 2014年11月20日
 *      Author: daniel
 */

#include "block.h"

static void gps_load(const struSize* size, void* param)
{
}

static void gps_unload(void)
{
}

static void gps_msg(Msg* msg)
{
}

const struBlockFunctions	block_gps =
{
	gps_load,
	gps_unload,
	gps_msg,
	NULL,
};
