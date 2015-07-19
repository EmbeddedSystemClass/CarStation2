/*
 * myRTC.h
 *
 *  Created on: Jan 6, 2015
 *      Author: daniel
 */

#ifndef MYRTC_H_
#define MYRTC_H_

#include <FreeRTOS.h>
#include "stm32f10x.h"
#include "..\shell\FreeRTOS_CLI.h"

BaseType_t	InitRTC(void);

extern const CLI_Command_Definition_t cmd_def_time;


#endif /* MYRTC_H_ */
