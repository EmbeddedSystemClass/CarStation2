/*
 * Pages.c
 *
 *  Created on: 2014年11月20日
 *      Author: daniel
 *
 *      用于定义页面的block组合，以及页面的按键行为
 */


#include "GUI.h"
#include "blocks/block.h"

// 记录当前页面位置，以及嵌套几层
#define PAGE_DEEPTH			5
static const struPage*	s_PageHistory[PAGE_DEEPTH];
static uint8_t		s_PagePos;


// 主页
static const struBlock		mainPage[] =
{
	{{0, 0, 0, 0}, &block_control, NULL},
	{{0, 9, 128, 32}, &block_clock, NULL},
	{{0,42, 128, 16}, &block_sh21, NULL},
	{{0,0, 128, 8}, &block_test, NULL}
};



// 页面组合
const struPage				Pages[] =
{
	{PAGE_DEBUG, 0, NULL, ACTION_BACK, ACTION_BACK, ACTION_NULL, ACTION_NULL},
	{PAGE_MAIN, sizeof(mainPage)/sizeof(mainPage[0]), mainPage, ACTION_BACK, ACTION_MSG, ACTION_MSG, ACTION_MSG},
};

// 根据页面名称（枚举）查找定义好的页面
const struPage* FindPage(enumPage name)
{
	int		i;

	for (i = 0; i < (int)(sizeof(Pages) / sizeof(Pages[0])); i++)
	{
		if (Pages[i].name == name)
		{
			return &Pages[i];
		}
	}

	return NULL;
}

void LoadFirstPage(enumPage name)
{
	int		i;

	// 初始化页面历史信息
	for (i = 0; i < PAGE_DEEPTH; i++)
	{
		s_PageHistory[i] = NULL;
	}
	s_PagePos = 0;

	s_PageHistory[0] = FindPage(name);
	if (s_PageHistory[0] == NULL)
	{
		// 如果指定的首页没有找到，就使用main页面（就是1）
		s_PageHistory[0] = &Pages[1];
	}

	// 加载页面
	LoadPage(s_PageHistory[0]);
}

void LoadPage(const struPage* page)
{
	int					i;
	LoadFunc_t			loadFunc;
	const struBlock*	block;

	for (i = 0; i < page->blockcount; i++)
	{
		block = &(page->blocks[i]);
		loadFunc = block->entry->load;
		if (loadFunc)
		{
			loadFunc(&(block->size), block->param);
		}
	}

	// 刷新ugfx
	gdispFlush();
}

void UnloadPage(const struPage* page)
{
	int						i;
	UnloadFunc_t			unloadFunc;
	const struBlock*		block;

	for (i = 0; i < page->blockcount; i++)
	{
		block = &(page->blocks[i]);
		unloadFunc = block->entry->unload;
		if (unloadFunc)
		{
			unloadFunc();
		}
	}

	// 刷新
	gdispFlush();
}

// 将消息发送给当前正在显示的页面的每个模块
void SendMsgToPage(Msg* msg)
{
	int					i;
	MsgFunc_t			msgFunc;
	const struBlock	*	block;
	const struPage*		page = s_PageHistory[s_PagePos];	// 当前页面，不做空指针判断，正常都应该不会是空指针的

	for (i = 0; i < page->blockcount; i++)
	{
		block = &(page->blocks[i]);
		msgFunc = block->entry->msg;
		if (msgFunc)
		{
			msgFunc(msg);
		}
	}

	// 刷新页面
	gdispFlush();
}

