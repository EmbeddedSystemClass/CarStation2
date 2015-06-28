/*
 * car_sh.c
 *
 *  Created on: 2014年10月1日
 *      Author: daniel
 */

#include <stm32f10x.h>
#include "car_sh.h"

#include "../SDCard/SDCard.h"
#include "../power/power.h"
#include "../LED/led.h"
#include "../GPS/GPs.h"
#include "../I2C/myi2c.h"
#include "../RTC/myRTC.h"

#define mainUART_COMMAND_CONSOLE_STACK_SIZE		( configMINIMAL_STACK_SIZE * 10 )
#define mainUART_COMMAND_CONSOLE_TASK_PRIORITY  	4
extern void vRegisterSampleCLICommands( void );
extern void vUARTCommandConsoleStart( uint16_t usStackSize, UBaseType_t uxPriority );

// 自定义命令入口
static const CLI_Command_Definition_t* commands[] = {

};

static const ShellCommand commands[] = {
	{"diskfree", cmd_diskfree},
//	{"mkfs", cmd_mkfs},
	{"sdinfo", cmd_sdinfo},
	{"dir", cmd_dir},
	{"power", cmd_power},
	{"chargeenable", cmd_chargeenable},
	{"led", cmd_led},
	{"gpsenable", cmd_gpsenable},
	{"light", cmd_light},
	{"time", cmd_time},
	{NULL, NULL}
};


bool_t InitShell(void)
{
	bool_t		bRet 	= TRUE;

	shellInit();

	// 注册Shell命令组
	vRegisterSampleCLICommands();

	// 创建Shell线程
	vUARTCommandConsoleStart(mainUART_COMMAND_CONSOLE_STACK_SIZE,
			mainUART_COMMAND_CONSOLE_TASK_PRIORITY);

	// 初始化Bluetooth的控制引脚
	EnableBluetooth(TRUE);

	return bRet;
}

void EnableBluetooth(bool_t bEnable)
{
	if (bEnable)
	{
		// 开启蓝牙（CarStation的串口1通过蓝牙模块输出，蓝牙模块的速率默认是115200）
		GPIO_SetBits(USARTsh_BT_GPIO, USARTsh_BT_Pin);
	}
	else
	{
		GPIO_ResetBits(USARTsh_BT_GPIO, USARTsh_BT_Pin);
	}
}


