/*
 * led.c
 *
 *  Created on: 2014年10月18日
 *      Author: daniel
 */

#include "led.h"

#include <string.h>

// 初始化所有LED引脚
BaseType_t InitLED( void )
{
	BaseType_t			ret = pdTRUE;
	GPIO_InitTypeDef 	GPIO_InitStructure;

	// Enable GPIO Peripheral clock
	RCC_APB2PeriphClockCmd(LED_INTERNAL_GPIO_CLK | LED_RED_GPIO_CLK | LED_GREEN_GPIO_CLK, ENABLE);

	// Configure pin in output push/pull mode
	GPIO_InitStructure.GPIO_Pin = LED_INTERNAL_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(LED_INTERNAL_GPIO, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = LED_RED_Pin;
	GPIO_Init(LED_RED_GPIO, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = LED_GREEN_Pin;
	GPIO_Init(LED_GREEN_GPIO, &GPIO_InitStructure);

	return ret;
}

static BaseType_t cmd_led( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	BaseType_t		bOn;
	const char *	pcParameter;
	BaseType_t 		xParameterStringLength;

	pcParameter = FreeRTOS_CLIGetParameter(pcCommandString, 2, &xParameterStringLength);
	configASSERT( pcParameter );
	bOn = (*pcParameter == '1') ? pdTRUE : pdFALSE;

	pcParameter = FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParameterStringLength);
	configASSERT( pcParameter );
	if (strcasecmp(pcParameter, "debug") == 0)
	{
		if (bOn)
			DEBUG_LED_ON;
		else
			DEBUG_LED_OFF;
	}
	else if (strcasecmp(pcParameter, "red") == 0)
	{
		if (bOn)
			RED_LED_ON;
		else
			RED_LED_OFF;
	}
	else if (strcasecmp(pcParameter, "green") == 0)
	{
		if (bOn)
			GREEN_LED_ON;
		else
			GREEN_LED_OFF;
	}

	return pdFALSE;
}

const CLI_Command_Definition_t cmd_def_led =
{
	"led",
	"\r\nled debug|red|gree 0|1 \r\n Control color LED.\r\n",
	cmd_led, /* The function to run. */
	2
};
