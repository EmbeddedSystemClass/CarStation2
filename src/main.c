//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stm32f10x.h"

// #include "SDCard/SDCard.h"
#include "GPS/GPS.h"
#include "shell/car_sh.h"
#include "I2C/i2cdevices.h"
#include "power/power.h"
#include "UI/GUI.h"
#include "main/controller.h"
#include "RTC/myRTC.h"
#include "Msg/Msg.h"
#include "Sensorsthread/SensorsThread.h"
#include "LED/led.h"

// ----------------------------------------------------------------------------
//
// Standalone STM32F1 empty sample (trace via NONE).
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the NONE output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

static void LEDTask( void * pvParameters)
{
	while (1)
	{
		vTaskDelay(500 / portTICK_RATE_MS);
		DEBUG_LED_ON;

		vTaskDelay(500 / portTICK_RATE_MS);
		DEBUG_LED_OFF;
	}
}

// Main task
#define MAIN_TASK_STACK_SIZE		1024
static void MainTask( void * pvParameters)
{
	//
	InitLED();

	// 初始化消息队列
	InitMsgQueue();

	//  Test LED task
	xTaskCreate( LEDTask, "Echo", configMINIMAL_STACK_SIZE, NULL, 1, NULL );

	// 初始化SD Card和mount文件系统
	//InitSDCard();

	// 初始化Shell
	//InitShell();
	//EnableBluetooth(pdTRUE);

	// 初始化GPS模块
	//InitGPS();

	// 初始化电源监控部分
	//InitPower();

	// 初始化I2C总线和相关设备
	//InitI2C();

	// 初始化GUI（内部会创建GUI线程）
	//InitGUI();

	// 初始化RTC，每秒一个中断
	//InitRTC();

	// 启动传感器测量线程
	//InitSensorsThread();

	// 进入controller主循环（不会再退出）
	controller_entry();

}

int
main(int argc, char* argv[])
{
	// At this stage the system clock should have already been configured
	// at high speed.
	// Main task
	xTaskCreate( MainTask, "Main", MAIN_TASK_STACK_SIZE, NULL, 1, NULL );

	//xTaskCreate( LEDTask, "Echo", configMINIMAL_STACK_SIZE, NULL, 1, NULL );

	// start FreeRTOS
	vTaskStartScheduler();

	// Infinite loop
	while (1)
	{
		// Add your code here.
	}
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	/* This function will get called if a task overflows its stack.   If the
	parameters are corrupt then inspect pxCurrentTCB to find which was the
	offending task. */

	( void ) pxTask;
	( void ) pcTaskName;

	for( ;; );
}
