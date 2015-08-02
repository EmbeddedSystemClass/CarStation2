/*
 * led.h
 *
 *  Created on: 2014年10月18日
 *      Author: daniel
 *      提供所有LED，包含调试用的和按键组合中的LED（目前只有调试模块的按键边上有一个双色的LED，这两个LED连接PWM引脚，可以用于做睡眠灯的慢闪模式）
 */

#ifndef LED_H_
#define LED_H_

#include <FreeRTOS.h>
#include "stm32f10x.h"
#include "..\shell\FreeRTOS_CLI.h"

#define LED_INTERNAL_GPIO           GPIOC
#define LED_INTERNAL_GPIO_CLK       RCC_APB2Periph_GPIOC
#define LED_INTERNAL_Pin            GPIO_Pin_5

#define LED_RED_GPIO           		GPIOC
#define LED_RED_GPIO_CLK       		RCC_APB2Periph_GPIOC
#define LED_RED_Pin           		GPIO_Pin_6

#define LED_GREEN_GPIO           	GPIOC
#define LED_GREEN_GPIO_CLK       	RCC_APB2Periph_GPIOC
#define LED_GREEN_Pin            	GPIO_Pin_7

#define DEBUG_LED_OFF		GPIO_ResetBits(LED_INTERNAL_GPIO, LED_INTERNAL_Pin)
#define DEBUG_LED_ON		GPIO_SetBits(LED_INTERNAL_GPIO, LED_INTERNAL_Pin)
//#define DEBUG_LED_TOGGLE	palTogglePad(GPIO_LED_INTERNAL_PORT, GPIO_LED_INTERNAL_BIT)

#define RED_LED_OFF			GPIO_ResetBits(LED_RED_GPIO, LED_RED_Pin)
#define RED_LED_ON			GPIO_SetBits(LED_RED_GPIO, LED_RED_Pin)
//#define RED_LED_TOGGLE		palTogglePad(GPIO_LED_RED_PORT, GPIO_LED_RED_BIT)

#define GREEN_LED_OFF		GPIO_ResetBits(LED_GREEN_GPIO, LED_GREEN_Pin)
#define GREEN_LED_ON		GPIO_SetBits(LED_GREEN_GPIO, LED_GREEN_Pin)
//#define GREEN_LED_TOGGLE	palTogglePad(GPIO_LED_GREEN_PORT, GPIO_LED_GREEN_BIT)

extern const CLI_Command_Definition_t cmd_def_led;

#endif /* LED_H_ */
