/*
 * block.h
 *
 *  Created on: 2014年11月20日
 *      Author: daniel
 */

#ifndef BLOCK_H_
#define BLOCK_H_

#include "../GUI.h"

typedef struct
{
	struSize		size;
	const void*	param;

	// 引用计数器
	uint8_t			refcount;		// 先初始化，再卸载，这样可以避免初始化获取数据较慢的问题。所以引入这个 引用计数器
} struBlockInfo;

// 所有block的函数、全局变量定义都在这里定义
extern const struBlockFunctions	block_clock;
extern const struBlockFunctions	block_gps;
extern const struBlockFunctions	block_sh21;
extern const struBlockFunctions	block_control;
extern const struBlockFunctions	block_test;

#endif /* BLOCK_H_ */
