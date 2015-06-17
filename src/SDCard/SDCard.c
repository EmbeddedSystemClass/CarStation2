/*
 * SDCard.c
 *
 *  Created on: 2014年9月14日
 *      Author: daniel
 */

#include <hal.h>
#include <ff.h>
#include <chprintf.h>

#include "SDCard.h"

FATFS SDC_FS;

bool_t InitSDCard(void)
{
	bool_t		bRet;
	FRESULT		err;

	bRet = FALSE;

	/*
	* Active SDC
	*/
	sdcStart(&SDCD1, NULL);
	if (sdcConnect(&SDCD1) == CH_SUCCESS)
	{
	  //mount filesystem
	  err = f_mount(0, &SDC_FS);

	  if (err != FR_OK)
	  {
		  bRet = FALSE;
		  sdcDisconnect(&SDCD1);
	  }
	  else
	  {
		  bRet = TRUE;
	  }
	}

	return bRet;
}


void cmd_diskfree(BaseSequentialStream *chp, int argc, char *argv[])
{
	(void)argv;
	(void)argc;

	FRESULT		err;
	uint32_t	clusters;
	uint64_t	freebytes;
	FATFS*		fsp;

	err = f_getfree("/", &clusters, &fsp);

	if (err != FR_OK)
	{
		chprintf(chp, "f_getfree failed, error:%d.\r\n", err);
		return;
	}

	freebytes = clusters * (uint32_t)SDC_FS.csize  / (2 * 1024);
	chprintf(chp, "FS: %lu free clusters, %lu sectors per cluster, %lu MB free\r\n",
			clusters, (uint32_t)SDC_FS.csize, freebytes);
}

//void cmd_mkfs(BaseSequentialStream *chp, int argc, char *argv[])
//{
//	(void)argv;
//	(void)argc;
//
//	FRESULT		err;
//
//	err = f_mkfs(0, 0, 0);
//
//	if (err != FR_OK)
//	{
//		chprintf(chp, "f_mkfs failed, error:%d.\r\n", err);
//	}
//	else
//	{
//		chprintf(chp, "f_mkfs successfully.\r\n");
//	}
//}

void cmd_sdinfo(BaseSequentialStream *chp, int argc, char *argv[])
{
	(void)argv;
	(void)argc;

	BlockDeviceInfo		sdinfo;
	if ( sdcGetInfo(&SDCD1, &sdinfo) == CH_SUCCESS )
	{
		chprintf(chp, "blk_num:%d, blk_size:%d.\r\n", sdinfo.blk_num, sdinfo.blk_size);
	}
	else
	{
		chprintf(chp, "sdcGetInfo failed.\r\n");
	}
}

void cmd_dir(BaseSequentialStream *chp, int argc, char *argv[])
{
	DIR		dir;
	char*	path;
	FRESULT	err;
	FILINFO fno;

	if (argc > 0)
	{
		path = argv[0];
	}
	else
	{
		path = "\0";
	}

	err = f_opendir(&dir, path);

	if (err == FR_OK)
	{
		while (true)
		{
			err = f_readdir(&dir, &fno);

			if ( (err != FR_OK) || (fno.fname[0] == 0) )
			{
				break;
			}

			if ( fno.fname[0] == '.')
			{
				continue;
			}

			if (fno.fattrib & AM_DIR)
			{
				// 目录
				chprintf(chp, "[%s]\r\n", fno.fname);
			}
			else
			{
				// 文件
				chprintf(chp, "%s\r\n", fno.fname);;
			}
		}
	}
	else
	{
		chprintf(chp, "f_opendir failed, error=%d.\r\n", err);
	}
}
