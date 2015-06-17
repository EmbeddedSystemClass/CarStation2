/*
 * GPS.h
 *
 *  Created on: 2014年9月14日
 *      Author: daniel
 */

#ifndef GPS_H_
#define GPS_H_

#include "ch.h"

bool_t InitGPS(void);
void EnableGPS(bool_t bEnable);

// shell commands
void cmd_gpsenable(BaseSequentialStream *chp, int argc, char *argv[]);
void cmd_gpsinfo(BaseSequentialStream *chp, int argc, char *argv[]);

#endif /* GPS_H_ */
