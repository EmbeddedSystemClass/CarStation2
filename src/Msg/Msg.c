/*
 * Msg.c
 *
 *  Created on: 2014年11月24日
 *      Author: daniel
 */


#include "Msg.h"

// 定义队列大小
QueueHandle_t		main_queue;
const unsigned portBASE_TYPE uxMainQueueSize = 20;

// 初始化消息队列
void InitMsgQueue(void)
{
	main_queue = xQueueCreate(uxMainQueueSize, sizeof(Msg*));
}


