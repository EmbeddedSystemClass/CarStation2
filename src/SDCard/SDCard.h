/*
 * SDCard.h
 *
 *  Created on: 2014年9月14日
 *      Author: daniel
 */

#ifndef SDCARD_H_
#define SDCARD_H_

#include <FreeRTOS.h>

BaseType_t InitSDCard(void);

void cmd_diskfree(BaseSequentialStream *chp, int argc, char *argv[]);
// void cmd_mkfs(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_sdinfo(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_dir(BaseSequentialStream *chp, int argc, char *argv[]);

#endif /* SDCARD_H_ */
