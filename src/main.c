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

#include "SDCard/SDCard.h"
#include "GPS/GPS.h"
#include "shell/car_sh.h"
#include "I2C/myi2c.h"
#include "power/power.h"
#include "UI/GUI.h"
#include "main/controller.h"
#include "RTC/myRTC.h"
#include "Msg/Msg.h"
#include "Sensorsthread/SensorsThread.h"

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

// Port numbers: 0=A, 1=B, 2=C, 3=D, 4=E, 5=F, 6=G, ...
#define BLINK_PORT_NUMBER               (1)
#define BLINK_PIN_NUMBER                (9)
#define BLINK_ACTIVE_LOW                (1)

#define BLINK_GPIOx(_N)                 ((GPIO_TypeDef *)(GPIOA_BASE + (GPIOB_BASE-GPIOA_BASE)*(_N)))
#define BLINK_PIN_MASK(_N)              (1 << (_N))
#define BLINK_RCC_MASKx(_N)             (RCC_APB2Periph_GPIOA << (_N))

inline void
__attribute__((always_inline))
blink_led_on(void)
{
#if (BLINK_ACTIVE_LOW)
  GPIO_ResetBits(BLINK_GPIOx(BLINK_PORT_NUMBER),
      BLINK_PIN_MASK(BLINK_PIN_NUMBER));
#else
  GPIO_SetBits(BLINK_GPIOx(BLINK_PORT_NUMBER),
      BLINK_PIN_MASK(BLINK_PIN_NUMBER));
#endif
}

inline void
__attribute__((always_inline))
blink_led_off(void)
{
#if (BLINK_ACTIVE_LOW)
  GPIO_SetBits(BLINK_GPIOx(BLINK_PORT_NUMBER),
      BLINK_PIN_MASK(BLINK_PIN_NUMBER));
#else
  GPIO_ResetBits(BLINK_GPIOx(BLINK_PORT_NUMBER),
      BLINK_PIN_MASK(BLINK_PIN_NUMBER));
#endif
}

static void LEDTask( void * pvParameters)
{
	// Enable GPIO Peripheral clock
	RCC_APB2PeriphClockCmd(BLINK_RCC_MASKx(BLINK_PORT_NUMBER), ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;

	// Configure pin in output push/pull mode
	GPIO_InitStructure.GPIO_Pin = BLINK_PIN_MASK(BLINK_PIN_NUMBER);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(BLINK_GPIOx(BLINK_PORT_NUMBER), &GPIO_InitStructure);

	while (1)
	{
		vTaskDelay(500 / portTICK_RATE_MS);
		blink_led_on();

		vTaskDelay(500 / portTICK_RATE_MS);
		blink_led_off();
	}
}

// Main task
static void MainTask( void * pvParameters)
{
	// Test LED task
	xTaskCreate( LEDTask, "Echo", configMINIMAL_STACK_SIZE, NULL, 1, NULL );

	// 初始化SD Card和mount文件系统
	//InitSDCard();

	// 初始化Shell
	//InitShell();
	EnableBluetooth(pdTRUE);

	// 初始化消息内存池
	//InitMsgMemoryPool();

	// 初始化GPS模块
	InitGPS();

	// 初始化电源监控部分
	InitPower();

	// 初始化I2C总线和相关设备
	InitI2C();

	// 初始化GUI（内部会创建GUI线程）
	InitGUI();

	// 初始化RTC，每秒一个中断
	InitRTC();

	// 启动传感器测量线程
	InitSensorsThread();

	// 进入controller主循环（不会再退出）
	controller_entry();

}

int
main(int argc, char* argv[])
{
	// At this stage the system clock should have already been configured
	// at high speed.
	// Main task
	xTaskCreate( MainTask, "Main", configMINIMAL_STACK_SIZE, NULL, 1, NULL );

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
