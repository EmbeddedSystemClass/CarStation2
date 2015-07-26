/*
 * power.c
 *
 *  Created on: 2014年10月6日
 *      Author: daniel
 */

#include "power.h"
#include <Msg/Msg.h>
#include "const.h"

#include <string.h>
#include <stdio.h>


#define ADC_GRP1_NUM_CHANNELS   3
#define ADC_GRP1_BUF_DEPTH      8

static uint16_t		voltages[ADC_GRP1_NUM_CHANNELS * ADC_GRP1_BUF_DEPTH];

// 缓存电压值（为判断方便，这里将电压数据换算完成）
static int16_t		c_CarBattery	= INVALID_VOLTAGE;	// 1/5取样
//static int16_t		c_CarAcc		= INVALID_VOLTAGE;	// 1/5取样

static BaseType_t		c_PowerOn		= pdFALSE;
static BaseType_t		c_CarRunning	= pdFALSE;
static BaseType_t		s_PowerIsChange	= pdFALSE;

#define ADC_REF_VOLTAGE			3300		// ADC参考电压
#define POWER_ON_VAL			5000		// 大于5v，就认为开启电源（有可能启动时，电池电压会跌倒很低）
#define CAR_RUNNING_VAL			13000		// 大于xx.x，认为启动，发电机工作


void PowerCheck(int16_t carBat, int16_t	carAcc);

BaseType_t InitPower(void)
{
	// 初始化各个控制引脚

	/*
	 *  Activates the ADC1 driver and the temperature sensor.
	 */
	//adcStart(&ADCD1, NULL);

	return pdTRUE;
}


/*
 * 对几个电压进行采样，提供shell使用，同步调用
 * Mode:        Linear buffer, 8 samples of 1 channel, SW triggered.
 * Channels:    IN10.
 */
//static const ADCConversionGroup adcgrpcfg1 = {
//  FALSE,
//  ADC_GRP1_NUM_CHANNELS,
//  NULL,
//  adcerrorcallback,
//  0, 0,                         /* CR1, CR2 */
//  ADC_SMPR1_SMP_AN10(ADC_SAMPLE_7P5) | ADC_SMPR1_SMP_AN11(ADC_SAMPLE_7P5) | ADC_SMPR1_SMP_AN12(ADC_SAMPLE_7P5),
//  0,                            /* SMPR2 */
//  ADC_SQR1_NUM_CH(ADC_GRP1_NUM_CHANNELS),
//  0,                            /* SQR2 */
//  ADC_SQR3_SQ1_N(ADC_CHANNEL_IN10) | ADC_SQR3_SQ2_N(ADC_CHANNEL_IN11) | ADC_SQR3_SQ3_N(ADC_CHANNEL_IN12)
//};

// 测量电压数据，和充电状态，以及充电允许状态，发送给主处理模块
// 在测量完成后，还对当前的电门和发电状态、电池电压状态，判断是否需要开启充电
void GetPowerStatus(void)
{
	int16_t		Vcar12bat, Vcar12acc;
	int32_t		vtemp;
	BaseType_t	err;

	// 获取电压，并对8次采样取平均值
	//err = adcConvert(&ADCD1, &adcgrpcfg1, voltages, ADC_GRP1_BUF_DEPTH);

	if (err == pdTRUE)
	{
		int8_t	i;

		// 取平均值
		Vcar12bat = Vcar12acc = 0;
		for (i = 0; i < ADC_GRP1_BUF_DEPTH; i++)
		{
			// 汽车12V电池
			Vcar12bat += voltages[i * ADC_GRP1_NUM_CHANNELS + 0];

			// 汽车电门输出（主要用于判断是否开启电源）
			Vcar12acc += voltages[i * ADC_GRP1_NUM_CHANNELS + 2];
		}
		Vcar12bat	/= ADC_GRP1_BUF_DEPTH;
		Vcar12acc	/= ADC_GRP1_BUF_DEPTH;

		// 进行电压转换和逻辑判断

		// 换算为实际电压（mv，整数）
		vtemp = 5 * ADC_REF_VOLTAGE * (int32_t)Vcar12bat / 4096;
		Vcar12bat = (int16_t)vtemp;
		vtemp = 5 * ADC_REF_VOLTAGE * (int32_t)Vcar12acc / 4096;
		Vcar12acc = (int16_t)vtemp;

		PowerCheck(Vcar12bat, Vcar12acc);

		if (s_PowerIsChange)
		{
			Msg*	msg;
			msg = MSG_NEW;
			if (msg)
			{
				msg->Id = MSG_POWER;
				msg->Param.PowerVoltage.CarBattery 	= c_CarBattery;
				msg->Param.PowerVoltage.IsPoweron	= c_PowerOn;
				msg->Param.PowerVoltage.IsCarStart	= c_CarRunning;

				err = MSG_SEND(msg);
				if (err != pdPASS)
				{
					MSG_FREE(msg);
				}
			}
		}


	}
}

void PowerCheck(int16_t carBat, int16_t	carAcc)
{
	BaseType_t		powerOn;
	BaseType_t		carRunning;

	// 检查是否应该启动充电，是否汽车开启电源，是否汽车启动了
	powerOn 	= (carAcc >= POWER_ON_VAL);
	carRunning 	= (carBat >= CAR_RUNNING_VAL) && powerOn;

	// 汽车启动状态变化
	if (carRunning != c_CarRunning)
	{
		s_PowerIsChange = pdTRUE;
		c_CarRunning = carRunning;
	}

	// 判断电压值是否变化，Poweron是否变化
	if ((c_PowerOn != powerOn) || (c_CarBattery != carBat) )
	{
		c_PowerOn = powerOn;
		c_CarBattery = carBat;

		s_PowerIsChange = pdTRUE;
	}
}

static BaseType_t cmd_power( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )

{
	(void)pcCommandString;
	(void)xWriteBufferLen;

	// 获取所有的电压值  （同步调用方式）
	//adcConvert(&ADCD1, &adcgrpcfg1, voltages, ADC_GRP1_BUF_DEPTH);

	// 输出电压值
	sprintf(pcWriteBuffer, "Car  Battery: %dmv\r\n", c_CarBattery);

	sprintf(pcWriteBuffer, "Car %s, %s\r\n", c_PowerOn ? "on" : "off", c_CarRunning ? "running" : "stop");

	return pdFALSE;
}

//void cmd_chargeenable(BaseSequentialStream *chp, int argc, char *argv[])
//{
//	// 开启或关闭充电
//	if (argc == 1)
//	{
//		// 有一个参数
//		if (*argv[0] == '1')
//		{
//			EnableCharge(true);
//			chprintf(chp, "Enable charge.\r\n");
//			return;
//		}
//		else if (*argv[0] == '0')
//		{
//			EnableCharge(false);
//			chprintf(chp, "Disable charge.\r\n");
//			return;
//		}
//	}
//
//	// usage
//	chprintf(chp, "chargeenable 1|0\r\n");
//}

const CLI_Command_Definition_t cmd_def_power =
{
	"power",
	"\r\npower \r\n Get battery status.\r\n",
	cmd_power, /* The function to run. */
	2
};
