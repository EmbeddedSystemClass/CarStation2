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
static BaseType_t cmd_time( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )

{
	uint32_t		now_sec;
	struct tm		now;
	const char *	pcParameter;
	BaseType_t 		xParameterStringLength;

	pcParameter = FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParameterStringLength);

	if (!pcParameter)
	{
		// 没有第一个参数，获取时间
		now_sec = RTC_GetCounter();
		localtime_r((time_t*)&now_sec, &now);

		sprintf(pcWriteBuffer, "time %d %d %d %d %d %d\r\n",
				now.tm_year + 1900,
				now.tm_mon + 1,
				now.tm_mday,
				now.tm_hour,
				now.tm_min,
				now.tm_sec);
	}
	else
	{
		// 设置时间（需要年、月、日、时、分，5个必须参数，秒可选）
		do
		{
			now.tm_year = atoi(pcParameter) - 1900;
			if (now.tm_year < 100)
			{
				sprintf(pcWriteBuffer, "Year wrong!\r\n");
				break;
			}

			pcParameter = FreeRTOS_CLIGetParameter(pcCommandString, 2, &xParameterStringLength);
			if (!pcParameter)
			{
				break;
			}
			now.tm_mon = atoi(pcParameter) - 1;
			if ( (now.tm_mon < 0) || (now.tm_mon > 12) )
			{
				sprintf(pcWriteBuffer, "Month wrong!\r\n");
				break;;
			}

			pcParameter = FreeRTOS_CLIGetParameter(pcCommandString, 3, &xParameterStringLength);
			if (!pcParameter)
			{
				break;
			}
			now.tm_mday = atoi(pcParameter);
			if (now.tm_mday > 31)
			{
				sprintf(pcWriteBuffer, "Day wrong!\r\n");
				break;;
			}

			pcParameter = FreeRTOS_CLIGetParameter(pcCommandString, 4, &xParameterStringLength);
			if (!pcParameter)
			{
				break;
			}
			now.tm_hour = atoi(pcParameter);
			if ( (now.tm_hour < 0) || (now.tm_hour > 23) )
			{
				sprintf(pcWriteBuffer, "Hour wrong!\r\n");
				break;
			}

			pcParameter = FreeRTOS_CLIGetParameter(pcCommandString, 5, &xParameterStringLength);
			if (!pcParameter)
			{
				break;
			}
			now.tm_min = atoi(pcParameter);
			if ( (now.tm_min < 0) || (now.tm_min > 59) )
			{
				sprintf(pcWriteBuffer, "Minute wrong!\r\n");
				break;
			}

			pcParameter = FreeRTOS_CLIGetParameter(pcCommandString, 2, &xParameterStringLength);
			if (pcParameter)
			{
				now.tm_sec = atoi(pcParameter);
				if ( (now.tm_sec < 0) || (now.tm_sec > 59) )
				{
					sprintf(pcWriteBuffer, "Second wrong!\r\n");
					break;
				}
			}
			else
			{
				now.tm_sec = 0;
			}

			/* Wait until last write operation on RTC registers has finished */
			RTC_WaitForLastTask();
			/* Change the current time */
			RTC_SetCounter(mktime(&now));
			/* Wait until last write operation on RTC registers has finished */
			RTC_WaitForLastTask();

			return pdFALSE;
		} while(0);

		// 参数不对
		sprintf(pcWriteBuffer, "Fail to set time.\r\n");
	}

	return pdFALSE;
}

const CLI_Command_Definition_t cmd_def_time =
{
	"time",
	"\r\ntime [yyyy mm dd hh mm [ss]] \r\n Get&Set RTC date&time.\r\n",
	cmd_time, /* The function to run. */
	-1
};



