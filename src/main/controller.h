/*
 * controller.h
 *
 *  Created on: 2014年11月18日
 *      Author: daniel
 */

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include <FreeRTOS.h>
#include <Msg/Msg.h>

typedef void (*MsgEntry_t)(Msg* msg);

typedef struct
{
	enumMsg		MsgId;
	MsgEntry_t	Entry;
} struEntry;

void controller_entry(void);

#endif /* CONTROLLER_H_ */
