/*
 * myRTC.c
 *
 *  Created on: Jan 6, 2015
 *      Author: daniel
 */

#include "myRTC.h"
#include <Msg/Msg.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

// RTC Interrupt
void RTC_IRQHandler(void)
{
	Msg*		msg;
	BaseType_t	ret;

	if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
	{
		/* Clear the RTC Second interrupt */
		RTC_ClearITPendingBit(RTC_IT_SEC);

		msg = MSG_NEW;
		if (msg)
		{
			msg->Id = MSG_RTC_SECOND;
			msg->Param.RTCSecond.time = RTC_GetCounter();
			ret = MSG_SEND_I(msg);
			if (ret != pdPASS)
			{
				MSG_FREE(msg);
			}
		}

		/* Wait until last write operation on RTC registers has finished */
		RTC_WaitForLastTask();

		// TODO:触发Event，用于传感器测量线程定时工作，采集数据给出处理线程
	}
}

// Configures RTC
void RTC_Configuration(void)
{
	/* Enable PWR and BKP clocks */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

	/* Allow access to BKP Domain */
	PWR_BackupAccessCmd(ENABLE);

	/* Reset Backup Domain */
	BKP_DeInit();

	/* Enable LSE */
	RCC_LSEConfig(RCC_LSE_ON);
	/* Wait till LSE is ready */
	while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
	{
	}

	/* Select LSE as RTC Clock Source */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

	/* Enable RTC Clock */
	RCC_RTCCLKCmd(ENABLE);

	/* Wait for RTC registers synchronization */
	RTC_WaitForSynchro();

	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();

	/* Enable the RTC Second */
	RTC_ITConfig(RTC_IT_SEC, ENABLE);

	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();

	/* Set RTC prescaler: set RTC period to 1sec */
	RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */

	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();
}

/**
  * @brief  Configures the nested vectored interrupt controller.
  * @param  None
  * @retval None
  */
void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Configure one bit for preemption priority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	/* Enable the RTC Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

BaseType_t InitRTC(void)
{
	/* NVIC configuration */
	NVIC_Configuration();

	if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
	{
		/* Backup data register value is not correct or not yet programmed (when
		 the first time the program is executed) */
		/* RTC Configuration */
		RTC_Configuration();

		// 1970 1 1 00:00:00
		RTC_SetCounter(0);

		BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
	}
	else
	{
		/* Wait for RTC registers synchronization */
		RTC_WaitForSynchro();

		/* Enable the RTC Second */
		RTC_ITConfig(RTC_IT_SEC, ENABLE);
		/* Wait until last write operation on RTC registers has finished */
		RTC_WaitForLastTask();
	}

	/* Clear reset flags */
	RCC_ClearFlag();
	return pdTRUE;
}

// 设置时间，获取时间（没有参数时）
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



