/*!
*****************************************************************************
** \file        adi/src/adi_vin.c
**
** \version     $Id: adi_vin.c 1 2014-08-11 09:24:58Z fengxuequan $
**
** \brief       ADI video input module function
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <basetypes.h>
#include "ambas_common.h"
#include "iav_drv.h"
#include "iav_drv_ex.h"
#include "ambas_vin.h"

#include "adi_types.h"
#include "adi_sys.h"
#include "adi_priv.h"
#include "adi_vin.h"


//*****************************************************************************
//*****************************************************************************
//** Local Defines
//*****************************************************************************
//*****************************************************************************
#define GADI_SETTING_CONTINUE               1
#define GADI_VIDEO_FPS_LIST_SIZE            256


//*****************************************************************************
//*****************************************************************************
//** Local structures
//*****************************************************************************
//*****************************************************************************


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
static GADI_VIN_HandleT* vinHandle;


//*****************************************************************************
//*****************************************************************************
//** Local Functions Declaration
//*****************************************************************************
//*****************************************************************************
static GADI_ERR vin_get_resolution(GADI_U32 vinMode, GADI_U32* vinWidth,
                                    GADI_U32* vinHeight);
static GADI_ERR vin_set_sensor_param(GADI_VIN_HandleT* handlePtr);
static GADI_ERR vin_check_source_info(GADI_VIN_HandleT* handlePtr,
                                    struct amba_vin_source_mode_info* modeInfo);
static GADI_ERR vin_set_params(GADI_VIN_HandleT* handlePtr,
                                    GADI_VIN_SettingParamsT* params);

//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************
GADI_ERR gadi_vin_init(void)
{
    if(vinHandle != NULL)
    {
        return GADI_VIN_ERR_ALREADY_INITIALIZED;
    }
    vinHandle = gadi_sys_malloc(sizeof(GADI_VIN_HandleT));
    if(vinHandle == NULL)
    {
        GADI_ERROR("gadi_vin_init: malloc memory failed!\n");
        return GADI_VIN_ERR_OUT_OF_MEMORY;
    }

    gadi_sys_memset(vinHandle, 0, sizeof(GADI_VIN_HandleT));

    GADI_INFO(" [DONE] Initialize VIN\n");

    return GADI_OK;
}

GADI_ERR gadi_vin_exit(void)
{
    if(vinHandle == NULL)
    {
        return GADI_VIN_ERR_NOT_INITIALIZED;
    }

    gadi_sys_free(vinHandle);
    vinHandle = NULL;

    return GADI_OK;
}

GADI_SYS_HandleT gadi_vin_open(GADI_VIN_OpenParamsT* openParams, GADI_ERR* errorCodePtr)
{
    GADI_U32 vinFps = 0;
    GADI_U32 width, height;
    GADI_U32 iavState;
    GADI_S32 iavFd;
    GADI_ERR retVal = GADI_OK;
    GADI_VIN_HandleT* handlePtr = NULL;
    struct amba_vin_source_info info;

    if((errorCodePtr == NULL) || (openParams == NULL))
    {
        *errorCodePtr = GADI_VIN_ERR_BAD_PARAMETER;
        return NULL;
    }

    if(vinHandle == NULL)
    {
        *errorCodePtr = GADI_VIN_ERR_NOT_INITIALIZED;
        return NULL;
    }

    handlePtr = vinHandle;

    /*set defualt parameters.*/
    retVal = gadi_priv_iav_get_fd(&iavFd);
    if(retVal != GADI_OK)
    {
        GADI_ERROR("gadi_vin_open: gadi_dsp_iav_get_fd: retVal =%d\n", retVal);
        *errorCodePtr = retVal;
        return NULL;
    }
    retVal = gadi_priv_iav_get_state(&iavState);
    if(retVal != GADI_OK)
    {
        GADI_ERROR("gadi_dsp_iav_get_state: retVal =%d\n", retVal);
        *errorCodePtr = retVal;
        return NULL;
    }

    if (iavState != GADI_DSP_IAV_STATE_IDLE)
    {
        GADI_ERROR("gadi_vin_open: error iav state state =%d\n", iavState);
        *errorCodePtr = GADI_VIN_ERR_FROM_DRIVER;
        return NULL;
    }

    handlePtr->fdIav       = iavFd;
    handlePtr->state       = iavState;
    handlePtr->mode        = openParams->mode;
    handlePtr->frameRate   = openParams->frameRate;
    handlePtr->antiFlicker = openParams->antiFlicker;
    handlePtr->sensorType  = openParams->sensorType;
    handlePtr->mirrorMode.mirrorPattern = openParams->mirrorMode.mirrorPattern;
    handlePtr->mirrorMode.bayerPattern  = openParams->mirrorMode.bayerPattern;

    if (ioctl(handlePtr->fdIav, IAV_IOC_VIN_SRC_GET_INFO, &info) < 0)
    {
        perror("IAV_IOC_VIN_SRC_GET_INFO error\n");
        *errorCodePtr = GADI_VIN_ERR_FROM_DRIVER;
        return NULL;
    }

    if (ioctl(handlePtr->fdIav, IAV_IOC_VIN_SRC_GET_FRAME_RATE, &vinFps) < 0)
    {
        perror("IAV_IOC_VIN_SRC_GET_FRAME_RATE error\n");
        *errorCodePtr = GADI_VIN_ERR_FROM_DRIVER;
        return NULL;
    }

    vin_get_resolution(info.default_mode,&width, &height);

    handlePtr->mode      = info.default_mode;
    handlePtr->vinWidth  = width;
    handlePtr->vinHeight = height;
    handlePtr->frameRate = vinFps;

    GADI_INFO("gadi_vin_open:[mode:%d] [%dx%d].\n", handlePtr->mode,
              handlePtr->vinWidth, handlePtr->vinHeight);

    *errorCodePtr = GADI_OK;

    return (GADI_SYS_HandleT)handlePtr;

}

GADI_ERR gadi_vin_close(GADI_SYS_HandleT handle)
{
    GADI_VIN_HandleT* vinHandleTemp = (GADI_VIN_HandleT*)handle;

    if(vinHandleTemp == NULL)
    {
        return GADI_VIN_ERR_BAD_PARAMETER;
    }

    gadi_sys_memset(vinHandleTemp, 0, sizeof(GADI_VIN_HandleT));

    return GADI_OK;
}

GADI_ERR gadi_vin_get_resolution(GADI_SYS_HandleT handle, GADI_U32* vinWidth, GADI_U32* vinHeight)
{
    GADI_VIN_HandleT* vinHandleTemp = (GADI_VIN_HandleT*)handle;

    if((vinHandleTemp == NULL) || (vinWidth == NULL) || (vinHeight == NULL))
    {
        return GADI_VIN_ERR_BAD_PARAMETER;
    }

    *vinWidth  = vinHandleTemp->vinWidth;
    *vinHeight = vinHandleTemp->vinHeight;

    return GADI_OK;
}

GADI_ERR gadi_vin_get_params(GADI_SYS_HandleT handle, GADI_VIN_SettingParamsT* vinSettingParams)
{
    GADI_VIN_HandleT* vinHandleTemp = (GADI_VIN_HandleT*)handle;

    if((vinHandleTemp == NULL) || (vinSettingParams == NULL))
    {
        return GADI_VIN_ERR_BAD_PARAMETER;
    }

    vinSettingParams->resoluMode = vinHandleTemp->mode;
    vinSettingParams->frameRate  = vinHandleTemp->frameRate;
    gadi_sys_memcpy(&(vinSettingParams->mirrorMode),
                    &(vinHandleTemp->mirrorMode), sizeof(GADI_VIN_MirrorModeParamsT));

    return GADI_OK;
}

GADI_ERR gadi_vin_set_params(GADI_SYS_HandleT handle, GADI_VIN_SettingParamsT* vinSettingParams)
{
    GADI_VIN_HandleT* vinHandleTemp = (GADI_VIN_HandleT*)handle;

    if((vinHandleTemp == NULL) || (vinSettingParams == NULL))
    {
        return GADI_VIN_ERR_BAD_PARAMETER;
    }

    if (vin_set_params(vinHandleTemp, vinSettingParams) < 0)
    {
        return GADI_VIN_ERR_FROM_DRIVER;
    }

    return GADI_OK;
}

GADI_ERR gadi_vin_enter_preview(GADI_SYS_HandleT handle, GADI_U32 enable)
{
    GADI_VIN_HandleT* vinHandleTemp = (GADI_VIN_HandleT*)handle;

    if((vinHandleTemp == NULL) || (vinHandleTemp->fdIav <= 0))
    {
        return GADI_VIN_ERR_BAD_PARAMETER;
    }

    if (enable)
    {
        if (ioctl(vinHandleTemp->fdIav, IAV_IOC_ENABLE_PREVIEW, 0) < 0)
        {
            perror("IAV_IOC_ENABLE_PREVIEW");
            return GADI_VIN_ERR_FROM_DRIVER;
        }

        vinHandleTemp->state = GADI_DSP_IAV_STATE_PREVIEW;
    }
    else
    {
        if (vinHandleTemp->state != GADI_DSP_IAV_STATE_PREVIEW)
        {
            GADI_INFO("gadi_vin_enter_preview:iav device state=%d\n", vinHandleTemp->state);
            return GADI_OK;
        }
        if (ioctl(vinHandleTemp->fdIav, IAV_IOC_ENTER_IDLE, 0) < 0)
        {
            perror("IAV_IOC_ENTER_IDLE");
            return GADI_VIN_ERR_FROM_DRIVER;
        }

        vinHandleTemp->state = GADI_DSP_IAV_STATE_IDLE;
    }

    return GADI_OK;
}


//*****************************************************************************
//*****************************************************************************
//** Local Functions
//*****************************************************************************
//*****************************************************************************
static GADI_ERR vin_get_resolution(GADI_U32 vinMode, GADI_U32* vinWidth, GADI_U32* vinHeight)
{
    GADI_U32 width, height;
    switch (vinMode)
    {
        case AMBA_VIDEO_MODE_5M_16_9:
            width = 2976;
            height = 1674;
            break;
        case AMBA_VIDEO_MODE_5M_4_3:
        case AMBA_VIDEO_MODE_QSXGA:
            width = 2592;
            height = 1944;
            break;
        case AMBA_VIDEO_MODE_2560x1440: // 3.7M, 16:9
            width = 2560;
            height = 1440;
            break;
        case AMBA_VIDEO_MODE_2304x1536:
            width = 2304;
            height = 1536;
            break;
        case AMBA_VIDEO_MODE_3M_16_9:       // 3.0M, 16:9
        case AMBA_VIDEO_MODE_2304x1296:
            width = 2304;
            height = 1296;
            break;
        case AMBA_VIDEO_MODE_QXGA:
        case AMBA_VIDEO_MODE_3M_4_3:
            width = 2048;
            height = 1536;
            break;
        case AMBA_VIDEO_MODE_2208x1242: // 2.7M, 16:9
            width = 2208;
            height = 1242;
            break;
        case AMBA_VIDEO_MODE_2048_1152: // 2.4M, 16:9
            width = 2048;
            height = 1152;
            break;
        case AMBA_VIDEO_MODE_1080I:
        case AMBA_VIDEO_MODE_1080I_PAL:
        case AMBA_VIDEO_MODE_1080P:
        case AMBA_VIDEO_MODE_1080P_PAL:
        case AMBA_VIDEO_MODE_1080I48:
        case AMBA_VIDEO_MODE_1080P24:
        case AMBA_VIDEO_MODE_1080P25:
        case AMBA_VIDEO_MODE_1080P30:
            width = 1920;
            height = 1080;
            break;
        case AMBA_VIDEO_MODE_UXGA:
            width = 1600;
            height = 1200;
            break;
        case AMBA_VIDEO_MODE_SXGA:
            width = 1280;
            height = 1024;
            break;
        case AMBA_VIDEO_MODE_1280_960:
            width = 1280;
            height = 960;
            break;
        case AMBA_VIDEO_MODE_WXGA:
            width = 1280;
            height = 800;
            break;
        case AMBA_VIDEO_MODE_720P:
        case AMBA_VIDEO_MODE_720P_PAL:
            width = 1280;
            height = 720;
            break;
        case AMBA_VIDEO_MODE_XGA:
            width = 1024;
            height = 768;
            break;
        case AMBA_VIDEO_MODE_576I:
        case AMBA_VIDEO_MODE_D1_PAL:
            width = 720;
            height = 576;
            break;
        case AMBA_VIDEO_MODE_480I:
        case AMBA_VIDEO_MODE_D1_NTSC:
            width = 720;
            height = 480;
            break;
        default:
            width = 1920;
            height = 1080;
            break;
    }

    *vinWidth  = width;
    *vinHeight = height;

    return GADI_OK;
}

/*
 *  The function is used to set 3A data, only for sensor that output RGB raw data
 */
static GADI_ERR vin_set_sensor_param(GADI_VIN_HandleT* handlePtr)
{
    GADI_S32    vinShutterTime = 60;
    GADI_S32    vinAgcDB       = 6;
    GADI_U32    shutterTimeQ9;
    GADI_S32    agcDB;

    if ((handlePtr == NULL) || (handlePtr->fdIav <= 0))
    {
        return GADI_VIN_ERR_BAD_PARAMETER;
    }

    shutterTimeQ9 = Q9_BASE / vinShutterTime;
    agcDB = vinAgcDB << 24;

    if (ioctl(handlePtr->fdIav, IAV_IOC_VIN_SRC_SET_SHUTTER_TIME, shutterTimeQ9) < 0)
    {
        perror("IAV_IOC_VIN_SRC_SET_SHUTTER_TIME");
        return GADI_VIN_ERR_FROM_DRIVER;
    }

    if (ioctl(handlePtr->fdIav, IAV_IOC_VIN_SRC_SET_AGC_DB, agcDB) < 0)
    {
        perror("IAV_IOC_VIN_SRC_SET_AGC_DB");
        return GADI_VIN_ERR_FROM_DRIVER;
    }

    return GADI_OK;
}


static GADI_ERR vin_check_source_info(GADI_VIN_HandleT* handlePtr,
                                  struct amba_vin_source_mode_info* modeInfo)
{
    struct amba_vin_source_info srcInfo;

    if ((handlePtr == NULL) || (handlePtr->fdIav <= 0) || (modeInfo == NULL))
    {
        return GADI_VIN_ERR_BAD_PARAMETER;
    }

    if (ioctl(handlePtr->fdIav, IAV_IOC_VIN_SRC_GET_INFO, &srcInfo) < 0)
    {
        perror("IAV_IOC_VIN_SRC_GET_INFO\n");
        return GADI_VIN_ERR_FROM_DRIVER;
    }

    if (srcInfo.dev_type != AMBA_VIN_SRC_DEV_TYPE_DECODER)
    {
        if (ioctl(handlePtr->fdIav, IAV_IOC_VIN_SRC_CHECK_VIDEO_MODE, modeInfo) < 0)
        {
            perror("IAV_IOC_VIN_SRC_CHECK_VIDEO_MODE\n");
            GADI_ERROR("vin_check_source_info: IAV_IOC_VIN_SRC_CHECK_VIDEO_MODE failed!\n");
            return GADI_SETTING_CONTINUE;
        }
        handlePtr->frameRate = modeInfo->video_info.fps;
    }

    return GADI_OK;
}

static GADI_ERR vin_set_params(GADI_VIN_HandleT* handlePtr, GADI_VIN_SettingParamsT* params)
{
    GADI_S32 i;
    GADI_S32 retVal;
    GADI_S32 sourceIndex = -1;
    GADI_U32 sourceNum = 0;
    GADI_U32 fpsList[GADI_VIDEO_FPS_LIST_SIZE];
    struct amba_video_info videoInfo;
    struct amba_vin_source_info sourceInfo;
    struct amba_vin_source_mode_info modeInfo;

    if ((handlePtr == NULL) || (handlePtr->fdIav <= 0) || (params == NULL))
    {
        return GADI_VIN_ERR_BAD_PARAMETER;
    }

    if (ioctl(handlePtr->fdIav, IAV_IOC_VIN_GET_SOURCE_NUM, &sourceNum) < 0)
    {
        perror("IAV_IOC_VIN_GET_SOURCE_NUM\n");
        return GADI_VIN_ERR_FROM_DRIVER;
    }
    if (sourceNum < 1)
    {
        printf("Please check sensor driver!\n");
        return GADI_VIN_ERR_FROM_DRIVER;
    }

    for (i = 0; i<sourceNum; i++)
    {
        if (ioctl(handlePtr->fdIav, IAV_IOC_VIN_SET_CURRENT_SRC, &i) < 0)
        {
            perror("IAV_IOC_VIN_SET_CURRENT_SRC\n");
            return GADI_VIN_ERR_FROM_DRIVER;
        }
        modeInfo.mode = handlePtr->mode;
        modeInfo.fps_table_size = GADI_VIDEO_FPS_LIST_SIZE;
        modeInfo.fps_table = fpsList;

        retVal = vin_check_source_info(handlePtr, &modeInfo);
        if(retVal == GADI_SETTING_CONTINUE)
        {
            continue;
        }
        else if(retVal < 0)
        {
            return GADI_VIN_ERR_FROM_DRIVER;
        }
        else
        {
            sourceIndex = i;
            break;
        }

    }

    if (sourceIndex == -1)
    {
        return GADI_VIN_ERR_FROM_DRIVER;
    }

    if (ioctl(handlePtr->fdIav, IAV_IOC_VIN_SET_CURRENT_SRC, &sourceIndex) < 0)
    {
        perror("IAV_IOC_VIN_SET_CURRENT_SRC");
        return GADI_VIN_ERR_FROM_DRIVER;
    }

    if (ioctl(handlePtr->fdIav, IAV_IOC_VIN_SRC_SET_VIDEO_MODE, params->resoluMode) < 0)
    {
        perror("IAV_IOC_VIN_SRC_SET_VIDEO_MODE");
        return GADI_VIN_ERR_FROM_DRIVER;
    }
    handlePtr->mode = params->resoluMode;

    if ((params->mirrorMode.mirrorPattern < 4) &&
        (params->mirrorMode.bayerPattern < 4))
    {
        if (ioctl(handlePtr->fdIav, IAV_IOC_VIN_SRC_SET_MIRROR_MODE, &(params->mirrorMode)) < 0)
        {
            perror("IAV_IOC_VIN_SRC_SET_MIRROR_MODE");
            return GADI_VIN_ERR_FROM_DRIVER;
        }
        else
        {
            handlePtr->mirrorMode.mirrorPattern = params->mirrorMode.mirrorPattern;
            handlePtr->mirrorMode.bayerPattern  = params->mirrorMode.bayerPattern;
        }
    }

    if (ioctl(handlePtr->fdIav, IAV_IOC_VIN_SRC_GET_INFO, &sourceInfo) < 0)
    {
        perror("IAV_IOC_VIN_SRC_GET_INFO");
        return GADI_VIN_ERR_FROM_DRIVER;
    }

    if (sourceInfo.dev_type != AMBA_VIN_SRC_DEV_TYPE_DECODER)
    {
        if (ioctl(handlePtr->fdIav, IAV_IOC_VIN_SRC_SET_FRAME_RATE, params->frameRate) < 0)
        {
            perror("IAV_IOC_VIN_SRC_SET_FRAME_RATE");
            return GADI_VIN_ERR_FROM_DRIVER;
        }
        handlePtr->frameRate = params->frameRate;
    }

    if (ioctl(handlePtr->fdIav, IAV_IOC_VIN_SRC_GET_VIDEO_INFO, &videoInfo) < 0)
    {
        perror("IAV_IOC_VIN_SRC_GET_VIDEO_INFO");
        return GADI_VIN_ERR_FROM_DRIVER;
    }
    handlePtr->vinWidth  = videoInfo.width;
    handlePtr->vinHeight = videoInfo.height;

    if (sourceInfo.dev_type != AMBA_VIN_SRC_DEV_TYPE_DECODER)
    {
        if (videoInfo.type == AMBA_VIDEO_TYPE_RGB_RAW)
        {
            if (vin_set_sensor_param(handlePtr) < 0)    //set aaa here
            {
                return GADI_VIN_ERR_FROM_DRIVER;
            }
        }
    }

    return 0;
}


