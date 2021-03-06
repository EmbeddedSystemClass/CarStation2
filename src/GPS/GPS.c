/*
 * GPS.c
 *
 *	GPS串口处理，判断是否定位，获取时间、经纬度坐标、速度、方向
 *  Created on: 2014年9月14日
 *      Author: daniel
 */

#include "GPS.h"
#include <string.h>
#include <stdio.h>
#include "Msg/Msg.h"

#include "semphr.h"
#include "task.h"

// GPS serial config(9600bps)
#define USARTgps                  USART2
#define USARTgps_GPIO             GPIOA
#define USARTgps_CLK              RCC_APB1Periph_USART2
#define USARTgps_GPIO_CLK         RCC_APB2Periph_GPIOA
#define USARTgps_RxPin            GPIO_Pin_3
#define USARTgps_TxPin            GPIO_Pin_2

// GPS使能控制引脚
#define USARTgps_GPS_GPIO         GPIOA
#define USARTgps_GPS_GPIO_CLK     RCC_APB2Periph_GPIOA
#define USARTgps_GPS_Pin          GPIO_Pin_1

#define			MAX_GPS_BUFFER		128
static char	gpsBuffer[MAX_GPS_BUFFER];		// 存放接收到的缓冲数据
static char*	gpsBufferPointer;				// 当前缓冲器的空白位置指针

static char	gpsCommand[MAX_GPS_BUFFER];		// 处理缓冲区，在串口接收到回车后，将接收缓冲区的字符串拷贝到这里
static int		nGPSCommandLength;				// 处理缓冲区长度。备注：0表示处理完成；不为0表示还有数据等待处理
												// 此时如果接收到回车，就应该丢弃接收的数据，不能覆盖处理。

// 触发命令处理的二值信号量
static SemaphoreHandle_t		s_xProcessEvent;

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

static BaseType_t getGPSCommand(void);

void USART2_IRQHandler(void)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	char cChar;

	if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
	{
		cChar = USART_ReceiveData(USART2);

		// 接收到的字符写入到缓冲区中
		if (gpsBufferPointer - gpsBuffer >= MAX_GPS_BUFFER)
		{
			// 缓冲区溢出，因为一直没有收到回车。超过MAX_GPS_BUFFER长度
			gpsBufferPointer = gpsBuffer;
		}

		*gpsBufferPointer = cChar;

		// 判断是否是\n，如果是就触发处理线程进行一次命令分析
		if (cChar == '\n')
		{
			// 拷贝接收缓冲区的数据到处理缓冲区
			if (nGPSCommandLength == 0)
			{
				nGPSCommandLength = gpsBufferPointer - gpsBuffer;
				memcpy(gpsCommand, gpsBuffer, nGPSCommandLength);

				xSemaphoreGiveFromISR(s_xProcessEvent, &xHigherPriorityTaskWoken);
			}
			else
			{
				// 处理缓冲区还有数据（异常）
				// 强制复位指针，丢弃接收缓冲区的数据
			}

			gpsBufferPointer = gpsBuffer;		// 复位接收数据的指针
		}
	}

	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}


void EnableGPS(BaseType_t bEnable)
{
	if (bEnable)
	{
		// 开启GPS模块
		GPIO_SetBits(USARTgps_GPS_GPIO, USARTgps_GPS_Pin);
		USART_Cmd(USARTgps, ENABLE);
	}
	else
	{
		GPIO_ResetBits(USARTgps_GPS_GPIO, USARTgps_GPS_Pin);
		USART_Cmd(USARTgps, DISABLE);
	}
}


// gps线程，检查是否需要开启GPS模块，以及处理GPS接收到的数据，然后发送到统一的处理模块
// 处理数据包含：时间、位置、车速、方位角、当前卫星数
#define GPS_TASK_STACK_SIZE			128
static void GPSTask(void * pvParameters)
{
	// test：开启GPS
	//EnableGPS(true);

	while (pdTRUE)
	{
		// 判断是否开启或关闭GPS（只在车上电源处于ACC，即供电状态，才启动）
		// TODO:

		// 等待二值信号量触发后，做一次命令分析
		xSemaphoreTake(s_xProcessEvent, portMAX_DELAY);

		// 处理数据

		// 分析缓冲器（多次分析，可能一个缓冲器内放入两个或以上的GPS指令）
		getGPSCommand();
	}
}

BaseType_t InitGPS(void)
{
	// 初始化GPS使用的串口
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// 初始化二值信号量
	vSemaphoreCreateBinary(s_xProcessEvent);

	// 初始化缓冲区位置指针
	gpsBufferPointer = gpsBuffer;

	// 初始化管脚
	/* Enable USARTgps Clock */
	RCC_APB1PeriphClockCmd(USARTgps_CLK, ENABLE);
	RCC_APB2PeriphClockCmd(USARTgps_GPIO_CLK | USARTgps_GPS_GPIO_CLK, ENABLE);
	/*!< GPIO configuration */
	/* Configure USARTgps Rx as input floating */
	GPIO_InitStructure.GPIO_Pin = USARTgps_RxPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(USARTgps_GPIO, &GPIO_InitStructure);

	/* Configure USARTgps Tx as alternate function pugps-pull */
	GPIO_InitStructure.GPIO_Pin = USARTgps_TxPin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(USARTgps_GPIO, &GPIO_InitStructure);

	// 蓝牙控制引脚
	GPIO_InitStructure.GPIO_Pin = USARTgps_GPS_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(USARTgps_GPS_GPIO, &GPIO_InitStructure);

	// 初始化UART
	/* USARTgps and USARTz configuration ------------------------------------------------------*/
	/* USARTgps and USARTz configured as follow:
	 - BaudRate = 230400 baud
	 - Word Length = 8 Bits
	 - One Stop Bit
	 - Even parity
	 - Hardware flow control disabled (RTS and CTS signals)
	 - Receive and transmit enabled
	 */
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl =
			USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	/* Configure USARTgps */
	USART_Init(USARTgps, &USART_InitStructure);

	//Enable the interrupt
	USART_ITConfig(USARTgps, USART_IT_RXNE, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// 启动GPS处理线程（要先启动线程，才能启动串口）
	xTaskCreate( GPSTask, "GPS", GPS_TASK_STACK_SIZE, NULL, 1, NULL );

	/* Enable the USARTgps */
	// 在独立的函数中控制

	return pdTRUE;
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

	p = gpsCommand + 1;	// 跳过$
	while (nLength)
	{
		c ^= *p++;
		nLength--;
	}

	// 判断计算结果
	p++;		// 移动到最后两个字符前
	snprintf(Checksum, 3, "%02X", c);

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

// 从缓冲区中分离命令
static BaseType_t getGPSCommand(void)
{
	BaseType_t		bHaveCommand = pdFALSE;

	// 分析命令的条件
	// 1、至少11个字符，头部6个，尾部3个字符的校验，2字符的回车换行
	// 2、头部为$
	if ((nGPSCommandLength > 11) && (gpsCommand[0] == '$'))
	{
		// 处理数据
		bHaveCommand = processGPSCommand(nGPSCommandLength);
	}

	// 处理完成
	nGPSCommandLength = 0;

	return bHaveCommand;
}


// 下面是shell命令函数

static BaseType_t cmd_gpsenable( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	// 开启或关闭GPS模块
	const char *	pcParameter;
	BaseType_t 		xParameterStringLength;

	pcParameter = FreeRTOS_CLIGetParameter(pcCommandString, 2, &xParameterStringLength);
	configASSERT( pcParameter );
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

