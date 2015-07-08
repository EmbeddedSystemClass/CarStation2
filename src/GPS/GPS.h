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

extern const CLI_Command_Definition_t cmd_def_gpsenable;
extern const CLI_Command_Definition_t cmd_def_gpsinfo;

#endif /* GPS_H_ */
