/*
 * i2c.h
 *
 *  Created on: 2014年10月28日
 *      Author: daniel
 */

#ifndef I2C_H_
#define I2C_H_

#include <FreeRTOS.h>

bool_t InitI2C(void);

// 读取车内外温度和湿度，如果有变化就发送到主处理模块
void GetTemperatureAndHumidity(void);

// 读取光照传感器数据
void GetLight(void);

void cmd_light(BaseSequentialStream *chp, int argc, char *argv[]);

#endif /* I2C_H_ */
