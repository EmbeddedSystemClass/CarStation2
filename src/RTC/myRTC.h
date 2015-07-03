/*
 * myRTC.h
 *
 *  Created on: Jan 6, 2015
 *      Author: daniel
 */

#ifndef MYRTC_H_
#define MYRTC_H_

#include <FreeRTOS.h>

BaseType_t	InitRTC(void);

void cmd_time(BaseSequentialStream *chp, int argc, char *argv[]);


#endif /* MYRTC_H_ */
