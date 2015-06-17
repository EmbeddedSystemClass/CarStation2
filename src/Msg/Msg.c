/*
 * Msg.c
 *
 *  Created on: 2014年11月24日
 *      Author: daniel
 */


#include "Msg.h"

// 定义Msg的Memory Pool数据
MEMORYPOOL_DECL(msgMP, sizeof(Msg), NULL);
static Msg		MsgArray[20];

// 初始化消息用的内存池
void InitMsgMemoryPool(void)
{
	// chPoolInit(&msgMP, sizeof(Msg), NULL);

	chPoolLoadArray(&msgMP, MsgArray, sizeof(MsgArray)/sizeof(MsgArray[0]));
}


