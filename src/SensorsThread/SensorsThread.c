/*
 * SensorsThread.c
 *
 *  Created on: Jan 3, 2015
 *      Author: daniel
 */


#include "SensorsThread.h"

#include "I2C/i2cdevices.h"
#include "power/power.h"

#include <task.h>

#define SENSORS_TASK_STACK_SIZE		256
static void SensorsTask( void * pvParameters)
{
	(void)pvParameters;

	// 循环定时读取各个传感器，然后将变化的数据Post到主线程
	while (1)
	{
		// Read every 1 seconds
		vTaskDelay(pdMS_TO_TICKS(1000));

		// Power(12v/5v/battery/door status)
		GetPowerStatus();

		// SH21(temperature, humidity)(inside and outside)
		GetTemperatureAndHumidity();

		// BH1750 (light)
		GetLight();

		//
	}
}

BaseType_t InitSensorsThread(void)
{
	BaseType_t		bRet;

	bRet = pdFALSE;

	// 创建传感器测量线程
	xTaskCreate( SensorsTask, "Sensors", SENSORS_TASK_STACK_SIZE, NULL, 1, NULL );

	return bRet;
}

