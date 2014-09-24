/******************************************************************************
** \file        adi/test/src/video.c
**
** \version     $Id: video.c 1615 2014-09-19 08:56:26Z fengxuequan $
**
** \brief       ADI layer video(video input/output encode) test.
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>


#include "video.h"

//*****************************************************************************
//*****************************************************************************
//** Local Defines
//*****************************************************************************
//*****************************************************************************
#define VIDEO_ENCODE_MODE  GADI_VENC_MODE_NORMAL

//*****************************************************************************
//*****************************************************************************
//** Local structures
//*****************************************************************************
//*****************************************************************************
typedef struct
{
    GADI_VENC_StreamFormatT streamFormat;
    GADI_VENC_H264ConfigT   h264Conf;
    GADI_VENC_MjpegConfigT  mjpegConf;
	GADI_U8					dptz;
}video_encode_stream;
//*****************************************************************************
//*****************************************************************************
//** Global Data
//*****************************************************************************
//*****************************************************************************


//*****************************************************************************
//*****************************************************************************
//** Local Data
//*****************************************************************************
//*****************************************************************************
static GADI_SYS_HandleT vinHandle   = NULL;
static GADI_SYS_HandleT voutHandle  = NULL;
static GADI_SYS_HandleT vencHandle  = NULL;

static video_encode_stream streams[GADI_VENC_STREAM_NUM];


//*****************************************************************************
//*****************************************************************************
//** Local Functions Declaration
//*****************************************************************************
//*****************************************************************************

static GADI_ERR check_encode_streams_params( GADI_U32 vinWidth, GADI_U32 vinHeight);
static GADI_ERR set_encode_stream_params(void);
static GADI_ERR calculate_encode_size_order(GADI_U32 *order, GADI_U32 streamsNum);
static GADI_ERR get_source_buffer_params(GADI_VENC_BufferFormatAllT *bufFormatAll,
                                           GADI_VENC_BufferTypeAllT *bufTypeAll);


//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************
GADI_ERR video_init(void)
{
    GADI_ERR retVal = GADI_OK;

    retVal = gadi_vin_init();
    retVal = gadi_vout_init();
    retVal = gadi_venc_init();

    return retVal;
}

GADI_ERR video_exit(void)
{
    GADI_ERR retVal = GADI_OK;

    retVal = gadi_vin_exit();
    retVal = gadi_vout_exit();
    retVal = gadi_venc_exit();

    return retVal;
}

GADI_ERR video_start_encode_stream(void)
{
    GADI_ERR retVal = GADI_OK;
    GADI_U32 cnt;
    GADI_U32 enable;
    GADI_U32 vinWidth, vinHeight;
    GADI_U8  encodeType;
    GADI_VIN_OpenParamsT  vinOpenParams;
    GADI_VOUT_OpenParamsT voutOpenParams;
    GADI_VENC_OpenParamsT vencOpenParams;

    GADI_VOUT_SettingParamsT voutParams;
    GADI_VIN_SettingParamsT  vinSettingParams;

    GADI_VENC_BufferTypeAllT bufTypeAll;
    GADI_VENC_BufferFormatAllT bufFormatAll;

    printf("video_start_encode_stream enter\n");
    /*set encode streams.*/
    set_encode_stream_params();

    /*open vin module.*/
    gadi_sys_memset(&vinOpenParams, 0, sizeof(GADI_VIN_OpenParamsT));
    vinOpenParams.mode = AMBA_VIDEO_MODE_AUTO;
    vinOpenParams.mirrorMode.mirrorPattern = 4;
    vinOpenParams.mirrorMode.bayerPattern  = 4;
    vinHandle = gadi_vin_open(&vinOpenParams, &retVal);
    if(retVal != GADI_OK)
    {
        printf("gadi_vin_open error\n");
        return retVal;
    }

    /*open vout module.*/
    gadi_sys_memset(&voutOpenParams, 0, sizeof(GADI_VOUT_OpenParamsT));
    voutOpenParams.resoluMode  = AMBA_VIDEO_MODE_480I;
    voutOpenParams.devFlag     = 1;
    voutOpenParams.sinkType    = AMBA_VOUT_SINK_TYPE_HDMI;
    voutOpenParams.cscEnable   = 1;
    voutOpenParams.videoEnable = 1;
    voutHandle = gadi_vout_open(&voutOpenParams, &retVal);
    if(retVal != GADI_OK)
    {
        printf("gadi_vout_open error\n");
        return retVal;
    }

    /*open venc module.*/
    gadi_sys_memset(&vencOpenParams, 0, sizeof(GADI_VENC_OpenParamsT));
    vencOpenParams.vinHandle  = vinHandle;
    vencOpenParams.voutHandle = voutHandle;
    vencHandle = gadi_venc_open(&vencOpenParams, &retVal);
    if(retVal != GADI_OK)
    {
        printf("gadi_venc_openerror\n");
        return retVal;
    }

    /*set VIN & VOUT & Source buffer*/
    enable = 0;
    retVal = gadi_vin_enter_preview(vinHandle, enable);
    if(retVal != GADI_OK)
    {
        printf("gadi_vin_enter_preview error\n");
        return retVal;
    }

    retVal = gadi_venc_set_encode_mode(vencHandle, VIDEO_ENCODE_MODE);
    if(retVal != GADI_OK)
    {
        printf("gadi_venc_set_encode_mode error\n");
        return retVal;
    }

    voutParams.resoluMode = AMBA_VIDEO_MODE_480I;
    voutParams.sinkType   = AMBA_VOUT_SINK_TYPE_HDMI;
    retVal = gadi_vout_set_params(voutHandle, &voutParams);
    if(retVal != GADI_OK)
    {
        printf("gadi_vout_set_params error\n");
        return retVal;
    }

    vinSettingParams.frameRate  = AMBA_VIDEO_FPS_29_97;
    vinSettingParams.resoluMode = AMBA_VIDEO_MODE_AUTO;
    vinSettingParams.mirrorMode.mirrorPattern = 4;
    vinSettingParams.mirrorMode.bayerPattern  = 4;
    retVal = gadi_vin_set_params(vinHandle, &vinSettingParams);
    if(retVal != GADI_OK)
    {
        printf("gadi_vin_set_params error\n");
        return retVal;
    }

    retVal = gadi_vin_get_resolution(vinHandle, &vinWidth, &vinHeight);
    if(retVal != GADI_OK)
    {
        printf("gadi_vin_get_resolution error\n");
        return retVal;
    }
    retVal = check_encode_streams_params(vinWidth, vinHeight);
    if(retVal != GADI_OK)
    {
        printf("check_encode_streams_params error\n");
        return retVal;
    }

    retVal = get_source_buffer_params(&bufFormatAll, &bufTypeAll);
    if(retVal != GADI_OK)
    {
        printf("get_source_buffer_params error\n");
        return retVal;
    }

    retVal = gadi_venc_set_buffer_format_all(vencHandle, &bufFormatAll);
    if(retVal != GADI_OK)
    {
        printf("set buffer format error.\n");
        return retVal;
    }

    retVal = gadi_venc_set_buffer_type_all(vencHandle, &bufTypeAll);
    if(retVal != GADI_OK)
    {
        printf("set buffer type error.\n");
        return retVal;
    }


	for (cnt = 0; cnt < GADI_VENC_STREAM_NUM; cnt++)
    {
		encodeType = streams[cnt].streamFormat.encodeType;
        if(encodeType != 0)
        {
			if (gadi_venc_set_stream_format(vencHandle, &streams[cnt].streamFormat) < 0)
            {
			    printf("gadi_venc_set_stream_format error, steamId=%d\n",cnt);
                return -1;
            }
            if(encodeType == GADI_VENC_TYPE_H264)
            {
                if(gadi_venc_set_h264_config(vencHandle, &streams[cnt].h264Conf) < 0)
                {
                    printf("gadi_venc_set_h264_config error, steamId=%d\n",cnt);
                    return -1;
                }
            }
            else if(encodeType == GADI_VENC_TYPE_MJPEG)
            {
                if(gadi_venc_set_mjpeg_config(vencHandle, &streams[cnt].mjpegConf) < 0)
                {
                    printf("gadi_venc_set_mjpeg_config error, steamId=%d\n",cnt);
                    return -1;
                }
            }
        }
    }

    enable = 1;
    if(gadi_vin_enter_preview(vinHandle, enable) < 0)
    {
        printf("gadi_vin_enter_preview enter error.\n");
        return -1;
    }

    /*start all streams encoding*/
    for (cnt = 0; cnt < GADI_VENC_STREAM_NUM; cnt++)
    {
		encodeType = streams[cnt].streamFormat.encodeType;
        if(encodeType != 0)
        {
            printf("start stream[%d]\n",cnt);
            if(gadi_venc_start_stream(vencHandle, cnt) < 0)
            {
                printf("gadi_venc_start_stream error, streams:%d\n", cnt);
                return -1;
            }
        }
    }

    return retVal;
}

GADI_ERR video_stop_encode_stream(void)
{
    GADI_ERR   retVal =  GADI_OK;


    return retVal;
}

//*****************************************************************************
//*****************************************************************************
//** Local Functions
//*****************************************************************************
//*****************************************************************************
static GADI_ERR check_encode_streams_params( GADI_U32 vinWidth, GADI_U32 vinHeight)
{
    GADI_ERR retVal = GADI_OK;
    GADI_U8  cnt;
    GADI_VENC_StreamFormatT *format = NULL;

    for(cnt=0; cnt<GADI_VENC_STREAM_NUM; cnt++)
    {
        format = &(streams[cnt].streamFormat);
        if((format->width > vinWidth) ||
           (format->height> vinHeight) ||
           (format->width + format->xOffset > vinWidth) ||
           (format->height + format->yOffset > vinHeight))
        {
            GADI_ERROR("encode stream[%d] resolution:%dX%d,x offset:%d, y offset:%d \
                is bigger than video input:%dX%d\n",
                cnt, format->width, format->height, format->xOffset, format->yOffset,
                vinWidth, vinHeight);

            format->xOffset = 0;
    		format->yOffset = 0;
    		format->width   = vinWidth;
    		format->height  = vinHeight;
        }
    }

    return retVal;
}

static GADI_ERR set_encode_stream_params(void)
{
    GADI_ERR retVal = GADI_OK;

    streams[0].dptz = 0;
    streams[0].streamFormat.streamId   = 0;
    streams[0].streamFormat.encodeType = GADI_VENC_TYPE_H264;
    streams[0].streamFormat.flipRotate = 0;
    streams[0].streamFormat.width      = 1920;
    streams[0].streamFormat.height     = 1080;
    streams[0].streamFormat.fps        = AMBA_VIDEO_FPS_30;
    streams[0].h264Conf.streamId       = 0;
    streams[0].h264Conf.gopM           = 1;
    streams[0].h264Conf.gopN           = 30;
    streams[0].h264Conf.gopModel       = 0;
    streams[0].h264Conf.idrInterval    = 1;
    streams[0].h264Conf.profile        = 0;
    streams[0].h264Conf.brcMode        = 0 ; //0:cbr, 1:vbr, 2: cbr keep quality, 3:vbr keep quality
    streams[0].h264Conf.cbrAvgBps      = 4000000;
    streams[0].h264Conf.vbrMinbps      = 1000000;
    streams[0].h264Conf.vbrMaxbps      = 4000000;
    streams[0].mjpegConf.streamId      = 0;
    streams[0].mjpegConf.quality       = 50;

    streams[1].dptz = 0;
    streams[1].streamFormat.streamId   = 1;
    streams[1].streamFormat.encodeType = GADI_VENC_TYPE_H264;
    streams[1].streamFormat.flipRotate = 1;
    streams[1].streamFormat.width      = 720;
    streams[1].streamFormat.height     = 480;
    streams[1].streamFormat.fps        = AMBA_VIDEO_FPS_30;
    streams[1].h264Conf.streamId       = 1;
    streams[1].h264Conf.gopM           = 1;
    streams[1].h264Conf.gopN           = 30;
    streams[1].h264Conf.gopModel       = 0;
    streams[1].h264Conf.idrInterval    = 1;
    streams[1].h264Conf.profile        = 0;
    streams[1].h264Conf.brcMode        = 0 ; //0:cbr, 1:vbr, 2: cbr keep quality, 3:vbr keep quality
    streams[1].h264Conf.cbrAvgBps      = 4000000;
    streams[1].h264Conf.vbrMinbps      = 1000000;
    streams[1].h264Conf.vbrMaxbps      = 4000000;
    streams[1].mjpegConf.streamId      = 1;
    streams[1].mjpegConf.quality       = 50;

    streams[2].dptz = 0;
    streams[2].streamFormat.streamId   = 2;
    streams[2].streamFormat.encodeType = GADI_VENC_TYPE_OFF;
    streams[2].streamFormat.flipRotate = 0;
    streams[2].streamFormat.width      = 0;
    streams[2].streamFormat.height     = 0;
    streams[2].streamFormat.fps        = 0;
    streams[2].h264Conf.streamId       = 2;
    streams[2].h264Conf.gopM           = 1;
    streams[2].h264Conf.gopN           = 30;
    streams[2].h264Conf.gopModel       = 0;
    streams[2].h264Conf.idrInterval    = 1;
    streams[2].h264Conf.profile        = 0;
    streams[2].h264Conf.brcMode        = 0 ; //0:cbr, 1:vbr, 2: cbr keep quality, 3:vbr keep quality
    streams[2].h264Conf.cbrAvgBps      = 4000000;
    streams[2].h264Conf.vbrMinbps      = 1000000;
    streams[2].h264Conf.vbrMaxbps      = 4000000;
    streams[2].mjpegConf.streamId      = 2;
    streams[2].mjpegConf.quality       = 50;

    streams[3].dptz = 0;
    streams[3].streamFormat.streamId   = 3;
    streams[3].streamFormat.encodeType = GADI_VENC_TYPE_OFF;
    streams[3].streamFormat.flipRotate = 0;
    streams[3].streamFormat.width      = 0;
    streams[3].streamFormat.height     = 0;
    streams[3].streamFormat.fps        = 0;
    streams[3].h264Conf.streamId       = 3;
    streams[3].h264Conf.gopM           = 1;
    streams[3].h264Conf.gopN           = 30;
    streams[3].h264Conf.gopModel       = 0;
    streams[3].h264Conf.idrInterval    = 1;
    streams[3].h264Conf.profile        = 0;
    streams[3].h264Conf.brcMode        = 0 ; //0:cbr, 1:vbr, 2: cbr keep quality, 3:vbr keep quality
    streams[3].h264Conf.cbrAvgBps      = 4000000;
    streams[3].h264Conf.vbrMinbps      = 1000000;
    streams[3].h264Conf.vbrMaxbps      = 4000000;
    streams[3].mjpegConf.streamId      = 3;
    streams[3].mjpegConf.quality       = 50;

    return retVal;
}

static GADI_ERR calculate_encode_size_order(GADI_U32 *order, GADI_U32 streamsNum)
{
	GADI_U32 tmp, resolution[GADI_VENC_STREAM_NUM];
	GADI_VENC_StreamFormatT *format;
	GADI_U32 i, j;

	for (i = 0; i < streamsNum; ++i)
    {
		format = &(streams[i].streamFormat);
		order[i] = i;
		if (format->encodeType == GADI_VENC_TYPE_OFF)
			resolution[i] = 0;
		else
			resolution[i] = (format->width << 16) + format->height;
	}
	for (i = 0; i < streamsNum; ++i)
    {
		for (j = i + 1; j < streamsNum; ++j)
        {
			if (resolution[i] < resolution[j])
            {
				tmp = resolution[j];
				resolution[j] = resolution[i];
				resolution[i] = tmp;
				tmp = order[j];
				order[j] = order[i];
				order[i] = tmp;
			}
		}
	}

	printf("resolution order of streams : ");
	for (i = 0; i < streamsNum; ++i)
    {
		printf("%d, ", order[i]);
	}
	printf("\n");

	return GADI_OK;
}

static GADI_ERR get_source_buffer_params(GADI_VENC_BufferFormatAllT *bufFormatAll,
                                           GADI_VENC_BufferTypeAllT *bufTypeAll)
{
    GADI_ERR retVal = GADI_OK;
	GADI_U32 cnt;
	GADI_U32 bufId;
    GADI_U32 usedFlag[GADI_VENC_STREAM_NUM];
	GADI_U32 order[GADI_VENC_STREAM_NUM];
	GADI_VENC_StreamFormatT *format = NULL;
    GADI_VOUT_SettingParamsT voutParams;
    GADI_VENC_BufferFormatAllT formatAll;

    if((bufFormatAll == NULL) || (bufTypeAll == NULL))
    {
        return -1;
    }

	for (cnt = 0; cnt < GADI_VENC_STREAM_NUM; cnt++)
    {
		usedFlag[cnt] = 0;
	}

    calculate_encode_size_order(order, GADI_VENC_STREAM_NUM);

    retVal = gadi_venc_get_buffer_format_all(vencHandle, &formatAll);
    if(retVal != GADI_OK)
    {
        return retVal;
    }

	for (cnt = 0; cnt < GADI_VENC_STREAM_NUM; cnt++)
    {
        format = &(streams[order[cnt]].streamFormat);
        bufId = GADI_VENC_BUFFER_MAIN;

        if (format->encodeType == GADI_VENC_TYPE_OFF)
        {
		    format->sourceBuf = GADI_VENC_BUFFER_MAIN;
			continue;
		}

		switch (format->height)
        {
            case 1080:
                bufFormatAll->mainWidth  = format->width;
                bufFormatAll->mainHeight = format->height;
                bufTypeAll->mainBufType  = GADI_VENC_SOURCE_BUFFER_TYPE_ENCODE;
                bufFormatAll->mainDeintlcForIntlcVin = formatAll.mainDeintlcForIntlcVin;
                bufId = GADI_VENC_BUFFER_MAIN;
                break;

            case 1024:
    		case 960:
    		case 720:
    		case 600:
                if ((usedFlag[GADI_VENC_BUFFER_MAIN] == 0) ||
                    (formatAll.mainHeight ==  format->height))
                {
                    bufFormatAll->mainWidth  = format->width;
                    bufFormatAll->mainHeight = format->height;
                    bufTypeAll->mainBufType  = GADI_VENC_SOURCE_BUFFER_TYPE_ENCODE;
                    bufFormatAll->mainDeintlcForIntlcVin = formatAll.mainDeintlcForIntlcVin;
                    bufId = GADI_VENC_BUFFER_MAIN;
                }
                else
                {
                    bufFormatAll->thirdWidth  = format->width;
                    bufFormatAll->thirdHeight = format->height;
                    bufTypeAll->thirdBufType  = GADI_VENC_SOURCE_BUFFER_TYPE_ENCODE;
                    bufFormatAll->thirdDeintlcForIntlcVin = formatAll.thirdDeintlcForIntlcVin;
                    bufId = GADI_VENC_BUFFER_THIRD;
                }
                break;

            case 576:
		    case 480:
                if ((usedFlag[GADI_VENC_BUFFER_MAIN] == 0) ||
                    (formatAll.mainHeight ==  format->height))
                {
                    bufFormatAll->mainWidth  = format->width;
                    bufFormatAll->mainHeight = format->height;
                    bufTypeAll->mainBufType  = GADI_VENC_SOURCE_BUFFER_TYPE_ENCODE;
                    bufFormatAll->mainDeintlcForIntlcVin = formatAll.mainDeintlcForIntlcVin;
                    bufId = GADI_VENC_BUFFER_MAIN;
                }
                else
                {
                    bufFormatAll->secondWidth  = format->width;
                    bufFormatAll->secondHeight = format->height;
                    bufTypeAll->secondBufType  = GADI_VENC_SOURCE_BUFFER_TYPE_ENCODE;
                    bufFormatAll->secondDeintlcForIntlcVin = formatAll.secondDeintlcForIntlcVin;
                    bufId = GADI_VENC_BUFFER_SECOND;
                }
                break;

            case 288:
    		case 240:
    		case 144:
    		case 120:
                if ((usedFlag[GADI_VENC_BUFFER_SECOND] == 0) ||
                    (formatAll.secondHeight ==  format->height))
                {
                    bufFormatAll->secondWidth  = format->width;
                    bufFormatAll->secondHeight = format->height;
                    bufTypeAll->secondBufType  = GADI_VENC_SOURCE_BUFFER_TYPE_ENCODE;
                    bufFormatAll->secondDeintlcForIntlcVin = formatAll.secondDeintlcForIntlcVin;
                    bufId = GADI_VENC_BUFFER_SECOND;
                }
                else if((usedFlag[GADI_VENC_BUFFER_THIRD] == 0) ||
                    (formatAll.thirdHeight ==  format->height))
                {
                    bufFormatAll->thirdWidth  = format->width;
                    bufFormatAll->thirdHeight = format->height;
                    bufTypeAll->thirdBufType  = GADI_VENC_SOURCE_BUFFER_TYPE_ENCODE;
                    bufFormatAll->thirdDeintlcForIntlcVin = formatAll.thirdDeintlcForIntlcVin;
                    bufId = GADI_VENC_BUFFER_SECOND;
                }
                else
                {
                    bufFormatAll->fourthWidth  = format->width;
                    bufFormatAll->fourthHeight = format->height;
                    bufTypeAll->fourthBufType  = GADI_VENC_SOURCE_BUFFER_TYPE_ENCODE;
                    bufFormatAll->fourthDeintlcForIntlcVin = formatAll.fourthDeintlcForIntlcVin;
                    bufId = GADI_VENC_BUFFER_FOURTH;
                }

                break;

            default:
                bufId = GADI_VENC_BUFFER_MAIN;
                GADI_DEBUG("special encode stream resolution:%dx%d\n",format->width, format->height);
			    break;

        }

        /*the buffer size != stream resolution???*/
        //format->xOffset??
        //format->yOffset??
        format->sourceBuf = bufId;
		usedFlag[bufId] = 1;
    }

    /*check 2nd source buffer. if not used, turn it off.*/
	if (usedFlag[GADI_VENC_BUFFER_SECOND] == 0)
    {
        bufFormatAll->secondWidth  = 0;
        bufFormatAll->secondHeight = 0;
        bufTypeAll->secondBufType  = GADI_VENC_SOURCE_BUFFER_TYPE_OFF;
    }

    /*if 3rd source buffer not used, set it as preview.*/
	if (usedFlag[GADI_VENC_BUFFER_THIRD] == 0)
    {
        retVal = gadi_vout_get_params(voutHandle, &voutParams);
        if(retVal != GADI_OK)
        {
            return retVal;
        }
		if (voutParams.sinkType == 0)
        {
            bufFormatAll->thirdWidth  = 0;
            bufFormatAll->thirdHeight = 0;
            bufTypeAll->thirdBufType  = GADI_VENC_SOURCE_BUFFER_TYPE_OFF;
		}
        else
        {
            bufFormatAll->thirdWidth  = 720;
            bufFormatAll->thirdHeight = 576;
            bufTypeAll->thirdBufType  = GADI_VENC_SOURCE_BUFFER_TYPE_PREVIEW;
            usedFlag[GADI_VENC_BUFFER_THIRD] = 1;
        }
	}

    /*check 2nd source buffer. if not used, turn it off.*/
	if (usedFlag[GADI_VENC_BUFFER_FOURTH] == 0)
    {
        bufFormatAll->fourthWidth  = 0;
        bufFormatAll->fourthHeight = 0;
        bufTypeAll->fourthBufType  = GADI_VENC_SOURCE_BUFFER_TYPE_OFF;
    }


    format = &(streams[0].streamFormat);
	if (format->width == 1920 &&
		format->height == 1080 &&
		(format->fps== AMBA_VIDEO_FPS_60 ||
		format->fps== AMBA_VIDEO_FPS_59_94))
    {
		bufFormatAll->intlcScan = 1;
    }
	else
    {
		bufFormatAll->intlcScan = 0;
    }

    return retVal;
}

