/*
 * SensorsThread.c
 *
 *  Created on: Jan 3, 2015
 *      Author: daniel
 */


#include "SensorsThread.h"

#include "I2C/myi2c.h"
#include "power/power.h"


static WORKING_AREA(sensorsThread, 256);
static msg_t sensors_Thread(void *arg);

BaseType_t InitSensorsThread(void)
{
	BaseType_t		bRet;

	bRet = FALSE;

	// 创建传感器测量线程
	chThdCreateStatic(sensorsThread, sizeof(sensorsThread), NORMALPRIO, sensors_Thread, NULL);

	return bRet;
}

static msg_t sensors_Thread(void *arg)
{
	(void)arg;

	// 循环定时读取各个传感器，然后将变化的数据Post到主线程
	while (TRUE)
	{
		// Read every 1 seconds
		chThdSleepMilliseconds(1000);

		// Power(12v/5v/battery/door status)
		GetPowerStatus();

		// SH21(temperature, humidity)(inside and outside)
		GetTemperatureAndHumidity();

		// BH1750 (light)
		GetLight();

		//
	}

	return 0;
}

