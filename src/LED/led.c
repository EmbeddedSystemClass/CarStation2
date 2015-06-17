/*
 * led.c
 *
 *  Created on: 2014年10月18日
 *      Author: daniel
 */

#include "led.h"
#include <hal.h>
#include <chprintf.h>
#include <string.h>

void cmd_led(BaseSequentialStream *chp, int argc, char *argv[])
{
	bool_t		bOn;

	// Usage: led debug|red|green 0|1

	if (argc < 2)
	{
		chprintf(chp, "Usage:led debug|red|gree 0|1 \r\n");
		return;
	}

	bOn = (*argv[1] == '1') ? true : false;

	if (strcasecmp(argv[0], "debug") == 0)
	{
		if (bOn)
			DEBUG_LED_ON;
		else
			DEBUG_LED_OFF;
	}
	else if (strcasecmp(argv[0], "red") == 0)
	{
		if (bOn)
			RED_LED_ON;
		else
			RED_LED_OFF;
	}
	else if (strcasecmp(argv[0], "green") == 0)
	{
		if (bOn)
			GREEN_LED_ON;
		else
			GREEN_LED_OFF;
	}
}
