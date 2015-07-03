/*
 * led.c
 *
 *  Created on: 2014年10月18日
 *      Author: daniel
 */

#include "led.h"

#include <string.h>

<<<<<<< HEAD
BaseType_t cmd_led( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
=======

static BaseType_t cmd_led( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )

>>>>>>> origin/master
void cmd_led(BaseSequentialStream *chp, int argc, char *argv[])
{
	BaseType_t		bOn;

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

<<<<<<< HEAD
const CLI_Command_Definition_t cmd_def_led =
{
	"led",
	"Usage:led debug|red|gree 0|1 \r\n",
	cmd_led, /* The function to run. */
	2 /* The user can enter any number of commands. */
};


=======
static const CLI_Command_Definition_t xThreeParameterEcho =
{
	"led",
	"\r\nled debug|red|gree 0|1 \r\n Control color LED.\r\n",
	cmd_led, /* The function to run. */
	2
};

>>>>>>> origin/master
