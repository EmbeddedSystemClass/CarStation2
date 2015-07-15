/*
 * i2c.c
 *
 *  Created on: 2014年10月28日
 *      Author: daniel
 */

#include "myi2c.h"
#include <hal.h>
#include <chprintf.h>
#include "Msg/Msg.h"

/* I2C1 */
static const I2CConfig i2cfg1 = {
    OPMODE_I2C,
    400000,
    FAST_DUTY_CYCLE_2,
};

/* I2C1 */
static const I2CConfig i2cfg2 = {
    OPMODE_I2C,
    400000,
    FAST_DUTY_CYCLE_2,
};

// I2C设备地址
#define SHT21_I2C_ADDR		0x40	// 在两个I2C都有

#define BH1750_I2C_ADDR		0x23	// I2C1

// GY80在I2C2上
#define BMP085_I2C_ADDR		0x77
#define ADXL345_I2C_ADDR	0x53
#define HMC5883L_I2C_ADDR	0x1E


// 缓存上次读取的数据，只有在有变化时，才将变化的数据发送到main主处理模块中，减少消息量
static uint16_t		c_Temperature_inside 	= (uint16_t)-1;
static uint16_t		c_Humidity_inside 		= (uint16_t)-1;
static uint16_t		c_Temperature_outside	= (uint16_t)-1;
static uint16_t		c_Humidity_outside		= (uint16_t)-1;

static uint16_t		c_Light					= 0;

BaseType_t InitI2C(void)
{
	BaseType_t	bRet = TRUE;

	 i2cStart(&I2CD1, &i2cfg1);
	 i2cStart(&I2CD2, &i2cfg2);

	return bRet;
}

void InitI2C()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	I2C_InitTypeDef I2C_InitStructure;

	// 初始化管脚
	/*!< sEE_I2C_SCL_GPIO_CLK and sEE_I2C_SDA_GPIO_CLK Periph clock enable */
	RCC_APB2PeriphClockCmd(sEE_I2C_SCL_GPIO_CLK | sEE_I2C_SDA_GPIO_CLK, ENABLE);

	/*!< GPIO configuration */
	/*!< Configure sEE_I2C pins: SCL */
	GPIO_InitStructure.GPIO_Pin = sEE_I2C_SCL_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_Init(sEE_I2C_SCL_GPIO_PORT, &GPIO_InitStructure);

	/*!< Configure sEE_I2C pins: SDA */
	GPIO_InitStructure.GPIO_Pin = sEE_I2C_SDA_PIN;
	GPIO_Init(sEE_I2C_SDA_GPIO_PORT, &GPIO_InitStructure);

	// 初始化I2C
	/*!< sEE_I2C Periph clock enable */
	RCC_APB1PeriphClockCmd(sEE_I2C_CLK, ENABLE);

	/*!< I2C configuration */
	/* sEE_I2C configuration */
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = I2C_SLAVE_ADDRESS7;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED;

	/* sEE_I2C Peripheral Enable */
	I2C_Cmd(sEE_I2C, ENABLE);
	/* Apply sEE_I2C configuration after enabling it */
	I2C_Init(sEE_I2C, &I2C_InitStructure);
}

#define Timed(x) Timeout = 0xFFFF; while (x) { if (Timeout-- == 0) goto errReturn;}

/*
 *  See AN2824 STM32F10xxx I2C optimized examples
 *
 *  This code implements polling based solution
 *
 */

/**
 *  Names of events used in stdperipheral library
 *
 *      I2C_EVENT_MASTER_MODE_SELECT                          : EV5
 *      I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED            : EV6
 *      I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED               : EV6
 *      I2C_EVENT_MASTER_BYTE_RECEIVED                        : EV7
 *      I2C_EVENT_MASTER_BYTE_TRANSMITTING                    : EV8
 *      I2C_EVENT_MASTER_BYTE_TRANSMITTED                     : EV8_2
 *
 **/

/*
 *  Read process is documented in RM008
 *
 *   There are three cases  -- read  1 byte  AN2824 Figure 2
 *                             read  2 bytes AN2824 Figure 2
 *                             read >2 bytes AN2824 Figure 1
 */

BaseType_t I2C_Read(I2C_TypeDef* I2Cx, uint8_t *buf, uint32_t nbyte, uint8_t SlaveAddress)
{
	__IO uint32_t Timeout = 0;

	//    I2Cx->CR2 |= I2C_IT_ERR;  interrupts for errors

	if (!nbyte)
		return pdTRUE;

	// Wait for idle I2C interface

	Timed(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));

	// Enable Acknowledgement, clear POS flag

	I2C_AcknowledgeConfig(I2Cx, ENABLE);
	I2C_NACKPositionConfig(I2Cx, I2C_NACKPosition_Current);

	// Intiate Start Sequence (wait for EV5

	I2C_GenerateSTART(I2Cx, ENABLE);
	Timed(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));

	// Send Address

	I2C_Send7bitAddress(I2Cx, SlaveAddress, I2C_Direction_Receiver);

	// EV6

	Timed(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_ADDR));

	if (nbyte == 1)
	{

		// Clear Ack bit

		I2C_AcknowledgeConfig(I2Cx, DISABLE);

		// EV6_1 -- must be atomic -- Clear ADDR, generate STOP

		__disable_irq();
		(void) I2Cx->SR2;
		I2C_GenerateSTOP(I2Cx, ENABLE);
		__enable_irq();

		// Receive data   EV7

		Timed(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_RXNE));
		*buf++ = I2C_ReceiveData(I2Cx);

	}
	else if (nbyte == 2)
	{
		// Set POS flag

		I2C_NACKPositionConfig(I2Cx, I2C_NACKPosition_Next);

		// EV6_1 -- must be atomic and in this order

		__disable_irq();
		(void) I2Cx->SR2;                           // Clear ADDR flag
		I2C_AcknowledgeConfig(I2Cx, DISABLE);       // Clear Ack bit
		__enable_irq();

		// EV7_3  -- Wait for BTF, program stop, read data twice

		Timed(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_BTF));

		__disable_irq();
		I2C_GenerateSTOP(I2Cx, ENABLE);
		*buf++ = I2Cx->DR;
		__enable_irq();

		*buf++ = I2Cx->DR;

	}
	else
	{
		(void) I2Cx->SR2;                           // Clear ADDR flag
		while (nbyte-- != 3)
		{
			// EV7 -- cannot guarantee 1 transfer completion time, wait for BTF
			//        instead of RXNE

			Timed(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_BTF));
			*buf++ = I2C_ReceiveData(I2Cx);
		}

		Timed(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_BTF));

		// EV7_2 -- Figure 1 has an error, doesn't read N-2 !

		I2C_AcknowledgeConfig(I2Cx, DISABLE);           // clear ack bit

		__disable_irq();
		*buf++ = I2C_ReceiveData(I2Cx);             // receive byte N-2
		I2C_GenerateSTOP(I2Cx, ENABLE);                  // program stop
		__enable_irq();

		*buf++ = I2C_ReceiveData(I2Cx);             // receive byte N-1

		// wait for byte N

		Timed(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED));
		*buf++ = I2C_ReceiveData(I2Cx);

		nbyte = 0;

	}

	// Wait for stop

	Timed(I2C_GetFlagStatus(I2Cx, I2C_FLAG_STOPF));
	return pdTRUE;

	errReturn:

	// Any cleanup here
	return pdFALSE;

}

/*
 * Read buffer of bytes -- AN2824 Figure 3
 */

BaseType_t I2C_Write(I2C_TypeDef* I2Cx, const uint8_t* buf, uint32_t nbyte,	uint8_t SlaveAddress)
{
	__IO uint32_t Timeout = 0;

	/* Enable Error IT (used in all modes: DMA, Polling and Interrupts */
	//    I2Cx->CR2 |= I2C_IT_ERR;
	if (nbyte)
	{
		Timed(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));

		// Intiate Start Sequence

		I2C_GenerateSTART(I2Cx, ENABLE);
		Timed(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));

		// Send Address  EV5

		I2C_Send7bitAddress(I2Cx, SlaveAddress, I2C_Direction_Transmitter);
		Timed(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

		// EV6

		// Write first byte EV8_1

		I2C_SendData(I2Cx, *buf++);

		while (--nbyte)
		{

			// wait on BTF

			Timed(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_BTF));
			I2C_SendData(I2Cx, *buf++);
		}

		Timed(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_BTF));
		I2C_GenerateSTOP(I2Cx, ENABLE);
		Timed(I2C_GetFlagStatus(I2C1, I2C_FLAG_STOPF));
	}
	return pdTRUE;
	errReturn: return pdFALSE;
}


// 读取照度传感器
BaseType_t ReadLightSensor(uint16_t* unLight)
{
	BaseType_t		bRet 	= false;
	msg_t 		status 	= RDY_OK;
	uint8_t		data[2];

	i2cAcquireBus(&I2CD1);
	do
	{
		// Power on
		data[0] = 1;
		status = i2cMasterTransmitTimeout(&I2CD1, BH1750_I2C_ADDR, data, 1, data, 0, MS2ST(4));
		if (status != RDY_OK)
		{
			break;
		}

		// Measure H2
		data[0] = 0x11;
		status = i2cMasterTransmitTimeout(&I2CD1, BH1750_I2C_ADDR, data, 1, data, 0, MS2ST(4));
		if (status != RDY_OK)
		{
			break;
		}

		// Delay
		chThdSleepMilliseconds(200);

		// Read result
		status = i2cMasterReceiveTimeout(&I2CD1, BH1750_I2C_ADDR, data, 2, MS2ST(10));
		if (status != RDY_OK)
		{
			break;
		}

		// 组合数据
		*unLight = ((uint16_t)(data[0]) << 8) | (uint8_t)(data[1]);

		bRet = true;
	} while (0);

	i2cReleaseBus(&I2CD1);

	return bRet;
}

void GetLight(void)
{
	uint16_t		light;
	Msg*			msg;
	msg_t			err;

	if (!ReadLightSensor(&light))
	{
		// read light failed
		return;
	}

	if (light == c_Light)
	{
		// no change
		return;
	}

	msg = MSG_NEW;
	if (!msg)
	{
		// malloc failed
		return;
	}

	msg->Id = MSG_LIGHT;
	msg->Param.Light.Light = light;

	err = MSG_SEND(msg);

	if (err == RDY_OK)
	{
		// update cache
		c_Light = light;
	}
	else
	{
		// send failed, free msg
		MSG_FREE(msg);
	}
}

uint8_t SHT21Checksum(uint8_t* pData, uint8_t length)
{
	// 计算SHT21的校验字
	uint8_t crc = 0;
	uint8_t byteCtr, bit;

	//calculates 8-Bit checksum with given polynomial
	for (byteCtr = 0; byteCtr < length; ++byteCtr)
	{
		crc ^= (pData[byteCtr]);
		for (bit = 8; bit > 0; --bit)
		{
			if (crc & 0x80)
				crc = (crc << 1) ^ 0x31;
			else
				crc = (crc << 1);
		}
	}

	return crc;
}

// 读取温湿度数据
BaseType_t ReadSHT21(I2CDriver* i2cp, uint16_t* pTemperature, uint16_t* pHumidity)
{
	BaseType_t		bRet = false;
	msg_t 		status 	= RDY_OK;
	uint8_t		data[3];
	uint8_t		i;

	i2cAcquireBus(i2cp);
	do
	{
		// 阻塞方式测量温度
//		data[0] = 0xE3;
//		status = i2cMasterTransmitTimeout(i2cp, SHT21_I2C_ADDR, data, 1, data, 3, MS2ST(200));
//		if (status != RDY_OK)
//		{
//			break;
//		}
		data[0] = 0xF3;
		status = i2cMasterTransmitTimeout(i2cp, SHT21_I2C_ADDR, data, 1, NULL, 0, MS2ST(10));
		if (status != RDY_OK)
		{
			break;
		}

		chThdSleepMilliseconds(100);

		for (i = 0; i < 10; i++)
		{
			status = i2cMasterReceiveTimeout(i2cp, SHT21_I2C_ADDR, data, 3, MS2ST(10));

			if (status == RDY_OK)
			{
				break;
			}
		}

		if (status != RDY_OK)
		{
			break;
		}

		// Check CRC
		if (data[2] != SHT21Checksum(data, 2))
		{
			break;
		}

		*pTemperature = data[0];
		*pTemperature = (*pTemperature << 8) + data[1];

		// 阻塞方式测量湿度
//		data[0] = 0xE5;
//		status = i2cMasterTransmitTimeout(i2cp, SHT21_I2C_ADDR, data, 1, data, 3, MS2ST(200));
//		if (status != RDY_OK)
//		{
//			break;
//		}

		data[0] = 0xF5;
		status = i2cMasterTransmitTimeout(i2cp, SHT21_I2C_ADDR, data, 1, NULL, 0, MS2ST(10));
		if (status != RDY_OK)
		{
			break;
		}

		chThdSleepMilliseconds(30);

		for (i = 0; i < 10; i++)
		{
			status = i2cMasterReceiveTimeout(i2cp, SHT21_I2C_ADDR, data, 3, MS2ST(10));

			if (status == RDY_OK)
			{
				break;
			}
		}

		if (status != RDY_OK)
		{
			break;
		}

		// check CRC
		if (data[2] != SHT21Checksum(data, 2))
		{
			break;
		}

		*pHumidity = data[0];
		*pHumidity = (*pHumidity << 8) + data[1];

		bRet = true;
	} while (0);
	i2cReleaseBus(i2cp);

	return bRet;
}

//#define HUMIDITY_CONVERT(h) (double)(-6.0 + (125 * (double)h) / 65536.0) * 100
#define HUMIDITY_CONVERT(h) (-600 + 3125 * h / 16384)
//#define TEMPERATURE_CONVERT(t) (double)(-46.85 + (double)(175.72 * (double)t) / 65536.0) * 100
#define TEMPERATURE_CONVERT(t) (-4685 + (4393 * t) / 16384)

// Last
void GetTemperatureAndHumidity(void)
{
	BaseType_t		bRet;
	uint16_t	unTemperature, unHumidity;
	Msg*		msg;
	msg_t		err;

	// inside
	bRet = ReadSHT21(&I2CD2, &unTemperature, &unHumidity);
	if (bRet)
	{
		// 判断是否有变化
		if ((unTemperature != c_Temperature_inside) || (unHumidity != c_Humidity_inside))
		{
			// Send Msg
			msg = MSG_NEW;
			if (msg)
			{
				msg->Id = MSG_SHT21_INSIDE;
				msg->Param.SHT21Data.Temperature = (int16_t)TEMPERATURE_CONVERT(unTemperature);
				msg->Param.SHT21Data.Humidity = (int16_t)HUMIDITY_CONVERT(unHumidity);

				// Send
				err = MSG_SEND(msg);
				if (err == RDY_OK)
				{
					// 成功时才更新内存缓存的值
					c_Temperature_inside = unTemperature;
					c_Humidity_inside = unHumidity;
				}
				else
				{
					// send failed, free msg
					MSG_FREE(msg);
				}
			}
		}
	}

	// outside
	bRet = ReadSHT21(&I2CD1, &unTemperature, &unHumidity);
	if (bRet)
	{
		// 判断是否有变化
		if ((unTemperature != c_Temperature_outside) || (unHumidity != c_Humidity_inside))
		{
			msg = MSG_NEW;
			if (msg)
			{
				msg->Id = MSG_SHT21_OUTSIDE;
				msg->Param.SHT21Data.Temperature = (int16_t)TEMPERATURE_CONVERT(unTemperature);
				msg->Param.SHT21Data.Humidity = (int16_t)HUMIDITY_CONVERT(unHumidity);

				err = MSG_SEND(msg);
				if (err == RDY_OK)
				{
					// update cache
					c_Temperature_outside = unTemperature;
					c_Humidity_outside = unHumidity;
				}
				else
				{
					MSG_FREE(msg);
				}
			}
		}
	}

}

// 读取数字罗盘

// 读取加速度传感器

// 读取陀螺仪传感器



// cmd
// 读取照度计数值
static BaseType_t cmd_light( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	(void)pcCommandString;
	(void)xWriteBufferLen;
	uint16_t		light = 0;


	if (ReadLightSensor(&light))
	{
		sprintf(pcWriteBuffer, "light = %d lux\r\n", light);
	}
	else
	{
		sprintf(pcWriteBuffer, "Read light failed.\r\n");
	}
}

const CLI_Command_Definition_t cmd_def_light =
{
	"light",
	"\r\nlight \r\n Get light sensor value.\r\n",
	cmd_light, /* The function to run. */
	2
};

