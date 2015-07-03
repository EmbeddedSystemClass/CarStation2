/*
 * GPS.h
 *
 *  Created on: 2014年9月14日
 *      Author: daniel
 */

#ifndef GPS_H_
#define GPS_H_

#include <FreeRTOS.h>

BaseType_t InitGPS(void);
void EnableGPS(BaseType_t bEnable);

// shell commands
void cmd_gpsenable(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_gpsinfo(BaseSequentialStream *chp, int argc, char *argv[]);

#endif /* GPS_H_ */
