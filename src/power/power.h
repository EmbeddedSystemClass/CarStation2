/*
 * power.h
 *
 *  Created on: 2014年10月6日
 *      Author: daniel
 */

#ifndef POWER_H_
#define POWER_H_

#include <FreeRTOS.h>
#include "stm32f10x.h"
#include "..\shell\FreeRTOS_CLI.h"

BaseType_t InitPower(void);
void EnableCharge(BaseType_t bEnable);
void GetPowerStatus(void);

extern const CLI_Command_Definition_t cmd_def_power;

#endif /* POWER_H_ */
