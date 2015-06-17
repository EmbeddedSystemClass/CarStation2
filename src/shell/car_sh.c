/*
 * car_sh.c
 *
 *  Created on: 2014年10月1日
 *      Author: daniel
 */

#include <stm32f10x.h>
#include <shell.h>
#include "car_sh.h"

#include "../SDCard/SDCard.h"
#include "../power/power.h"
#include "../LED/led.h"
#include "../GPS/GPs.h"
#include "../I2C/myi2c.h"
#include "../RTC/myRTC.h"

// 自定义命令入口
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

static const ShellConfig shell_cfg1 = {
  (BaseSequentialStream *)&SD1,
  commands
};

// shell thread statck size
static WORKING_AREA(waShell, 512);


bool_t InitShell(void)
{
	bool_t		bRet 	= TRUE;
	Thread *	shelltp = NULL;

	/*
	 * Activates the serial driver 1 using the driver default configuration.
	 */
	sdStart(&SD1, NULL);

	/*
	 * * Shell manager initialization.
	 */
	shellInit();

	// 创建Shell线程
	shelltp = shellCreateStatic(&shell_cfg1, waShell, sizeof(waShell), NORMALPRIO);

	// 初始化Bluetooth的控制引脚

	return bRet;
}

void EnableBluetooth(bool_t bEnable)
{
	if (bEnable)
	{
		// 开启蓝牙（CarStation的串口1通过蓝牙模块输出，蓝牙模块的速率默认是115200）
		palSetPad(GPIO_BLUETOOTH_PORT, GPIO_BLUETOOTH_BIT);
	}
	else
	{
		palClearPad(GPIO_BLUETOOTH_PORT, GPIO_BLUETOOTH_BIT);
	}
}
