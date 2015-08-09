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

#define ADC1_DR_Address    ((uint32_t)0x4001244C)

#define ADC_GRP1_NUM_CHANNELS   2
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
	GPIO_InitTypeDef 	GPIO_InitStructure;
	ADC_InitTypeDef 	ADC_InitStructure;
	DMA_InitTypeDef   	DMA_InitStructure;

	// 初始化ADC时钟
	RCC_ADCCLKConfig(RCC_PCLK2_Div2);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC, ENABLE);

	// 初始化各个控制引脚（PC0和PC2，因为安全问题，后续不再使用内部的锂电池，所以不需要PC1）
	// PC0 - ADC1-10 12v汽车电池（直通）
	// PC1 - ADC1-11 4.7v锂电池
	// PC2 - ADC1-12 12v电门（汽车钥匙在ON位置后才有电）
	// PC3 - 门灯信号（开门 低压？）（暂时使用输入，如果信号不能为低电平，则需要使用AD转换方式检测）
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	// 开门信号，低电平表示开门（使用中断方式）
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	// 初始化DMA
	/* DMA1 channel1 configuration ----------------------------------------------*/
	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) voltages;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = sizeof(voltages);
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal; //DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);

	// 初始化ADC1
	/* ADC1 configuration ------------------------------------------------------*/
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 2;
	ADC_Init(ADC1, &ADC_InitStructure);

	/* ADC1 regular channel0 & 12 configuration */
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_7Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 2, ADC_SampleTime_7Cycles5);

	/* Enable ADC1 DMA */
	ADC_DMACmd(ADC1, ENABLE);

	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);

	/* Enable ADC1 reset calibration register */
	ADC_ResetCalibration(ADC1);
	/* Check the end of ADC1 reset calibration register */
	while (ADC_GetResetCalibrationStatus(ADC1))
		;

	/* Start ADC1 calibration */
	ADC_StartCalibration(ADC1);
	/* Check the end of ADC1 calibration */
	while (ADC_GetCalibrationStatus(ADC1))
		;

	/* Start ADC1 Software Conversion */
	//ADC_SoftwareStartConvCmd(ADC1, ENABLE);

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
			Vcar12acc += voltages[i * ADC_GRP1_NUM_CHANNELS + 1];
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

const CLI_Command_Definition_t cmd_def_power =
{
	"power",
	"\r\npower \r\n Get battery status.\r\n",
	cmd_power, /* The function to run. */
	2
};
