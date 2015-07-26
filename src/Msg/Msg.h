/*
 * Msg.h
 *
 *  Created on: 2014年11月18日
 *      Author: daniel
 *      消息格式定义
 */

#ifndef MSG_H_
#define MSG_H_

#include <FreeRTOS.h>
#include <queue.h>

// 消息ID，主处理线程和GUI处理线程，以及其他处理线程，共用一个消息枚举，消息内存也一起合并定义
typedef enum
{
	MSG_RTC_SECOND,
	MSG_GPS,
	MSG_SHT21_INSIDE,
	MSG_SHT21_OUTSIDE,
	MSG_POWER,
	MSG_DOOR,
	MSG_LIGHT,

	// GUI
	MSG_UI_CLOCK,
	MSG_UI_TANDH_IN,
	MSG_UI_TANDH_OUT,
	MSG_UI_LIGHT,

	MSG_UI_ONOFF,
	MSG_UI_CONTRAST,
} enumMsg;

typedef union
{
	//Power voltage
	struct	Msg_Power
	{
		int16_t		CarBattery;

		int16_t		IsPoweron : 1;
		int16_t		IsCarStart : 1;
	}	PowerVoltage;

	struct Msg_DoorOpen
	{
		int16_t		IsOpen : 1;
	}	DoorOpen;

	// 温度和湿度
	struct Msg_SHT21
	{
		int16_t		Temperature;
		int16_t		Humidity;
	}	SHT21Data;

	struct Msg_GPS_Pos
	{
	}	GPSPos;

	struct Msg_Light
	{
		uint16_t	Light;
	}	Light;

	struct Msg_RTCSecond
	{
		uint32_t	time;
	} RTCSecond;

	// 下面定义给UI使用的消息（UI和主处理模块也可能共用消息结构，共用部分在上面）
	// 实时时钟消息（每分钟更新一次）
	struct Msg_UIClock
	{
		uint16_t	Year;
		uint8_t		Month;
		uint8_t		Day;

		uint8_t		Hour;
		uint8_t		Minute;
	} UIClock;

	// 温湿度实际数据，有变化时发送
	struct Msg_TandH
	{
		int16_t		Temperature;
		int16_t		Humidity;
	} TandH;

	// 液晶屏控制消息
	struct Msg_DisplayOnOff
	{
		int16_t		IsOn : 1;
	} DisplayOnOff;

	struct Msg_DisplayContrast
	{
		uint8_t		Contrast;
	} DisplayContrast;

} Msg_Param;

typedef struct
{
	enumMsg		Id;
	Msg_Param	Param;
} Msg;

void InitMsgQueue(void);

#define MSG_NEW			(Msg*)pvPortMalloc(sizeof(Msg))

#define MSG_FREE(p)		vPortFree((void*)p)

#define MSG_SEND(p)		xQueueSendToBack(&main_queue, (void*)p, portMAX_DELAY)
#define MSG_SEND_I(p)	xQueueSendToBackFromISR(&main_queue, (void*)p, pdFALSE)

#define GUI_MSG_SEND(p)		xQueueSendToBack(&gui_queue, (void*)p, portMAX_DELAY)
#define GUI_MSG_SEND_I(p)	xQueueSendToBackFromISR(&gui_queue, (void*)p, pdFALSE)

// 每个queue外部定义
extern QueueHandle_t		gui_queue;
extern QueueHandle_t		main_queue;
extern QueueHandle_t		log_queue;
extern QueueHandle_t		file_queue;

#endif /* MSG_H_ */
