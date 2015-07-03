/*
 * car_sh.h
 *
 *  Created on: 2014年10月1日
 *      Author: daniel
 *      car_sh主要提供CarStation的shell初始化和自定义shell的入口合并注册。
 *      每个自定义命令则在对应模块的代码文件中实现，不在此处实现
 */

#ifndef CAR_SH_H_
#define CAR_SH_H_

#include <FreeRTOS.h>
/* FreeRTOS+CLI includes. */
#include "FreeRTOS_CLI.h"
#include "UARTAdapter.h"

BaseType_t InitShell(void);

void EnableBluetooth(BaseType_t bEnable);

#endif /* CAR_SH_H_ */
