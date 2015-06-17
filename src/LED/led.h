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

#define DEBUG_LED_OFF		palClearPad(GPIO_LED_INTERNAL_PORT, GPIO_LED_INTERNAL_BIT)
#define DEBUG_LED_ON		palSetPad(GPIO_LED_INTERNAL_PORT, GPIO_LED_INTERNAL_BIT)
#define DEBUG_LED_TOGGLE	palTogglePad(GPIO_LED_INTERNAL_PORT, GPIO_LED_INTERNAL_BIT)

#define RED_LED_OFF			palClearPad(GPIO_LED_RED_PORT, GPIO_LED_RED_BIT)
#define RED_LED_ON			palSetPad(GPIO_LED_RED_PORT, GPIO_LED_RED_BIT)
#define RED_LED_TOGGLE		palTogglePad(GPIO_LED_RED_PORT, GPIO_LED_RED_BIT)

#define GREEN_LED_OFF		palClearPad(GPIO_LED_GREEN_PORT, GPIO_LED_GREEN_BIT)
#define GREEN_LED_ON		palSetPad(GPIO_LED_GREEN_PORT, GPIO_LED_GREEN_BIT)
#define GREEN_LED_TOGGLE	palTogglePad(GPIO_LED_GREEN_PORT, GPIO_LED_GREEN_BIT)

void cmd_led(BaseSequentialStream *chp, int argc, char *argv[]);

#endif /* LED_H_ */
