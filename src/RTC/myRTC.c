/*
 * myRTC.c
 *
 *  Created on: Jan 6, 2015
 *      Author: daniel
 */

#include "myRTC.h"
#include <hal.h>
#include <Msg/Msg.h>
#include <time.h>
#include <chprintf.h>
#include <stdlib.h>

static void rtc_cb(RTCDriver *rtcp, rtcevent_t event)
{
	RTCTime		timespec;
	Msg*		msg;
	msg_t		err;

	switch (event)
	{
	case RTC_EVENT_OVERFLOW:
	case RTC_EVENT_ALARM:
		break;
	case RTC_EVENT_SECOND:
		// 读取RTC，每秒发送一次（判断分钟由主处理模块完成，主处理模块还利用这个1秒消息构建一个秒级的定时器）
		chSysLockFromIsr();
		rtcGetTimeI(rtcp, &timespec);
		msg = MSG_NEW_I;
		if (msg)
		{
			msg->Id = MSG_RTC_SECOND;
			msg->Param.RTCSecond.time = timespec.tv_sec;
			err = MSG_SEND_I(msg);
			if (err != RDY_OK)
			{
				MSG_FREE_I(msg);
			}
		}
		chSysUnlockFromIsr();

		// 触发Event，用于传感器测量线程定时工作，采集数据给出处理线程
		// TODO

		break;

	default:
		break;
	}
}

BaseType_t InitRTC(void)
{
	// Set RTC callback
	rtcSetCallback(&RTCD1, rtc_cb);
	return true;
}

// 设置时间，获取时间（没有参数时）
void cmd_time(BaseSequentialStream *chp, int argc, char *argv[])
{
	RTCTime		timespec;
	struct tm	now;

	if (argc == 0)
	{
		// 获取时间
		rtcGetTime(&RTCD1, &timespec);
		localtime_r((time_t*)&timespec.tv_sec, &now);

		chprintf(chp, "time %d %d %d %d %d %d\r\n",
				now.tm_year + 1900,
				now.tm_mon + 1,
				now.tm_mday,
				now.tm_hour,
				now.tm_min,
				now.tm_sec);

		return;
	}
	else
	{
		// 设置时间（需要年、月、日、时、分，5个必须参数，秒可选）
		if ((argc < 5) || (argc > 6))
		{
			// usage
			chprintf(chp, "time yyyy mm dd hh mm [ss]\r\n");
			return;
		}

		now.tm_year = atoi(argv[0]) - 1900;
		if (now.tm_year < 100)
		{
			chprintf(chp, "Year wrong!\r\n");
			return;
		}
		now.tm_mon = atoi(argv[1]) - 1;
		if ( (now.tm_mon < 0) || (now.tm_mon > 12) )
		{
			chprintf(chp, "Month wrong!\r\n");
			return;
		}
		now.tm_mday = atoi(argv[2]);
		if (now.tm_mday > 31)
		{
			chprintf(chp, "Day wrong!\r\n");
			return;
		}
		now.tm_hour = atoi(argv[3]);
		if ( (now.tm_hour < 0) || (now.tm_hour > 23) )
		{
			chprintf(chp, "Hour wrong!\r\n");
			return;
		}
		now.tm_min = atoi(argv[4]);
		if ( (now.tm_min < 0) || (now.tm_min > 59) )
		{
			chprintf(chp, "Minute wrong!\r\n");
			return;
		}
		if (argc == 6)
		{
			now.tm_sec = atoi(argv[5]);
			if ( (now.tm_sec < 0) || (now.tm_sec > 59) )
			{
				chprintf(chp, "Second wrong!\r\n");
				return;
			}
		}

		timespec.tv_sec = mktime(&now);
		timespec.tv_msec = 0;
		rtcSetTime(&RTCD1, &timespec);

	}
}


