/*
 * GPS.c
 *
 *	GPS串口处理，判断是否定位，获取时间、经纬度坐标、速度、方向
 *  Created on: 2014年9月14日
 *      Author: daniel
 */

#include <hal.h>
#include <chprintf.h>
#include "GPS.h"
#include <string.h>
#include <stdio.h>
#include "Msg/Msg.h"

// GPS serial config(9600bps)
static SerialConfig		GPSConfig =
{
  9600,
  0,
  USART_CR2_STOP1_BITS | USART_CR2_LINEN,
  0
};

#define		MAX_GPS_BUFFER		128
static char	gpsBuffer[MAX_GPS_BUFFER];		// 存放接收到的缓冲数据
static char*	gpsBufferPointer;				// 当前缓冲器的空白位置指针

// GPS原始数据（用字符串存放，不需要处理就可以判断是否变化，有变化后在进行字符串转数字，再发送到处理线程上）
// 所有数据都要判断各自的有效性

// 定位模式（来自GSA）
static char	cGPSStatus[1];

// 经纬度、时间、日期（来自RMC）
// 经纬度
static char	cLatitude[10];
static char	cNS[1];

static char	cLongitude[10];
static char	cEW[1];

// 时间
static char	cTime[6];

// 日期
static char	cDate[6];

// 方位角、车速（来自VTG）
// 方位角

// 车速

// 海拔高度、卫星个数（来自GGA）
// 海拔高度

// 卫星个数

static WORKING_AREA(gpsThread, 128);
static msg_t gps_Thread(void *arg);
static BaseType_t getGPSCommand(void);

BaseType_t InitGPS(void)
{
	BaseType_t	bRet;

	bRet = pdTRUE;

	// 启动GPS处理线程
	chThdCreateStatic(gpsThread, sizeof(gpsThread), NORMALPRIO, gps_Thread, NULL);


	return bRet;
}
void EnableGPS(BaseType_t bEnable)
{
	if (bEnable)
	{
		// 开启GPS模块
		palSetPad(GPIO_GPS_PORT, GPIO_GPS_BIT);
		sdStart(&SD2, &GPSConfig);
	}
	else
	{
		palClearPad(GPIO_GPS_PORT, GPIO_GPS_BIT);
		sdStop(&SD2);
	}
}


// gps线程，检查是否需要开启GPS模块，以及处理GPS接收到的数据，然后发送到统一的处理模块
// 处理数据包含：时间、位置、车速、方位角、当前卫星数
static msg_t gps_Thread(void *arg) {

  (void)arg;

  int	nRead;

  chRegSetThreadName("gps");

  // test：开启GPS
  //EnableGPS(true);
  gpsBufferPointer = gpsBuffer;


  while (pdTRUE) {
	  // 判断是否开启或关闭GPS（只在车上电源处于ACC，即供电状态，才启动）
	  // TODO:


	  // 开始读取串口2数据
	  nRead = chnReadTimeout(&SD2, (unsigned char*)gpsBufferPointer, (gpsBuffer + sizeof(gpsBuffer) - gpsBufferPointer), MS2ST(100));
	  if (nRead > 0)
	  {
		  gpsBufferPointer = gpsBuffer + nRead;
		  // 处理数据

		  // 分析缓冲器（多次分析，可能一个缓冲器内放入两个或以上的GPS指令）
		  getGPSCommand();
	  }

    // chThdSleepMilliseconds(500);
  }

  return 0;
}

BaseType_t GPSCommandChecksum(int nLength)
{
	int	c		= 0;
	char*			p;
	char			Checksum[3];

	// 从缓冲区头部开始
	nLength -= 6;	// 头部的$、最后3个是校验字和"\r\n"

	if (nLength <= 0)
	{
		return pdFALSE;
	}

	p = gpsBuffer + 1;	// 跳过$
	while (nLength)
	{
		c ^= *p++;
		nLength--;
	}

	// 判断计算结果
	p++;		// 移动到最后两个字符前
	chsnprintf(Checksum, 3, "%02X", c);

	if ((Checksum[0] == *p++) && (Checksum[1] == *p))
	{
		return pdTRUE;
	}
	else
	{
		return pdFALSE;
	}
}

// 根据命令字，来处理GPS命令
BaseType_t processGPSCommand(int nLength)
{
	// 做校验
	if (!GPSCommandChecksum(nLength))
	{
		return pdFALSE;
	}

	// 判断$GP后的3个字符串
	// switch

	return pdFALSE;
}

// 清除缓冲区前面无用或已经使用的数据
void MoveData(char* pStart)
{
	int		nDataLength = gpsBufferPointer - pStart;
	memmove(gpsBuffer, pStart, nDataLength);
	gpsBufferPointer = gpsBuffer + nDataLength;
}

// 查找尾部，返回从头到尾的总字符串长度（包含$和回车换行）
int SearchEOF(void)
{
	char* p;
	for (p = gpsBuffer; p < gpsBufferPointer; p++)
	{
		if (*p == '\n')
		{
			return p - gpsBuffer + 1;
		}
	}

	return 0;
}
// 从缓冲区中分离命令
static BaseType_t getGPSCommand(void)
{
	BaseType_t		bHaveCommand = pdFALSE;

	while (pdFALSE)
	{
		if (gpsBufferPointer - gpsBuffer < 11)
		{
			// 没有足够数据（至少11个字符，头部6个，尾部3个字符的校验，2字符的回车换行）
			break;
		}

		//判断头部是否是$
		if ( gpsBuffer[0] == '$' )
		{
			// 查找尾部
			int nCommandLength;

			nCommandLength = SearchEOF();

			if (nCommandLength == 0)
			{
				break;
			}
			else
			{
				// 处理命令
				processGPSCommand(nCommandLength);

				bHaveCommand = pdTRUE;

				// 可能还有另一个命令，需要继续向下搜索
				MoveData(gpsBuffer + nCommandLength);
				continue;
			}
		}
		else
		{
			// 找缓冲区中的$然后移动
			char*		p;

			for (p = gpsBuffer; p < gpsBufferPointer; p++)
			{
				if (*p == '$')
				{
					MoveData(p);
					break;
				}
			}

			if (*p != '$')
			{
				// 没有找到$在缓冲区中，缓冲的数据全部无效
				gpsBufferPointer = gpsBuffer;
				break;
			}
		}
	}

	return bHaveCommand;
}

static BaseType_t cmd_gpsenable( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	// 开启或关闭GPS模块
	BaseType_t		bOn;
	const char *	pcParameter;
	BaseType_t 		xParameterStringLength;

	pcParameter = FreeRTOS_CLIGetParameter(pcCommandString, 2, &xParameterStringLength);
	configASSERT( pcParameter );
	bOn = (pcParameter == '1') ? pdTRUE : pdFALSE;

	if (pcParameter[0] == '1')
	{
		EnableGPS(pdTRUE);
		sprintf(pcWriteBuffer, "Enable GPS.\r\n");
	}
	else
	{
		EnableGPS(pdFALSE);
		sprintf(pcWriteBuffer, "Disable GPS.\r\n");
	}

	return pdFALSE;
}

const CLI_Command_Definition_t cmd_def_gpsenable =
{
	"gpsenable",
	"\r\ngpsenable 0|1 \r\n Control GPS enable/disable.\r\n",
	cmd_gpsenable,
	1
};


static BaseType_t cmd_gpsinfo( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	// 输出当前GPS的信息
	return pdFALSE;
}

const CLI_Command_Definition_t cmd_def_gpsinfo =
{
	"gpsinfo",
	"\r\ngpsinfo\r\n Get GPS information.\r\n",
	cmd_gpsinfo,
	0
};

