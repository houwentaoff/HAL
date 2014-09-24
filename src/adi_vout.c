/*!
*****************************************************************************
** \file        adi/src/adi_vout.c
**
** \version     $Id: adi_vout.c 1 2014-08-11 09:24:58Z fengxuequan $
**
** \brief       ADI video output module function
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
#include "ambas_vout.h"

#include "adi_types.h"
#include "adi_sys.h"
#include "adi_priv.h"
#include "adi_vout.h"


//*****************************************************************************
//*****************************************************************************
//** Local Defines
//*****************************************************************************
//*****************************************************************************


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
static GADI_VOUT_HandleT* voutHandle;

static GADI_VOUT_ResolutionT voutRes[] =
{
    // Typically for Analog and HDMI
    {"480i",        AMBA_VIDEO_MODE_480I,           720,    480},
    {"576i",        AMBA_VIDEO_MODE_576I,           720,    576},
    {"480p",        AMBA_VIDEO_MODE_D1_NTSC,        720,    480},
    {"576p",        AMBA_VIDEO_MODE_D1_PAL,         720,    576},
    {"720p",        AMBA_VIDEO_MODE_720P,           1280,   720},
    {"720p50",      AMBA_VIDEO_MODE_720P_PAL,       1280,   720},
    {"1080i",       AMBA_VIDEO_MODE_1080I,          1920,   1080},
    {"1080i50",     AMBA_VIDEO_MODE_1080I_PAL,      1920,   1080},
    {"1080p",       AMBA_VIDEO_MODE_1080P,          1920,   1080},
    {"1080p50",     AMBA_VIDEO_MODE_1080P_PAL,      1920,   1080},
    {"1080p30",     AMBA_VIDEO_MODE_1080P30,        1920,   1080},

};


//*****************************************************************************
//*****************************************************************************
//** Local Functions Declaration
//*****************************************************************************
//*****************************************************************************
static GADI_ERR vout_get_resolution(GADI_U32 mode, GADI_S32 *width,
                                    GADI_S32 *height);
static GADI_ERR vout_check_params(GADI_VOUT_HandleT* handlePtr,
                                    GADI_VOUT_SettingParamsT* params);
static GADI_ERR vout_get_sink_id(GADI_VOUT_HandleT* handlePtr, GADI_U32 sinkType,
                                    GADI_U32* sinkId);
static GADI_ERR vout_setup_params(GADI_VOUT_HandleT* handlePtr,
                                    GADI_VOUT_SettingParamsT* params);


//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************
GADI_ERR gadi_vout_init(void)
{
    if(voutHandle != NULL)
    {
        return GADI_VOUT_ERR_ALREADY_INITIALIZED;
    }
    voutHandle = gadi_sys_malloc(sizeof(GADI_VOUT_HandleT));
    if(voutHandle == NULL)
    {
        GADI_ERROR("gadi_vout_init: malloc memory failed!\n");
        return GADI_VOUT_ERR_OUT_OF_MEMORY;
    }

    gadi_sys_memset(voutHandle, 0, sizeof(GADI_VOUT_HandleT));


    GADI_INFO(" [DONE] Initialize VOUT\n");

    return GADI_OK;
}

GADI_ERR gadi_vout_exit(void)
{
    if(voutHandle == NULL)
    {
        return GADI_VOUT_ERR_NOT_INITIALIZED;
    }

    gadi_sys_free(voutHandle);
    voutHandle = NULL;

    return GADI_OK;
}

GADI_SYS_HandleT gadi_vout_open(GADI_VOUT_OpenParamsT* openParams, GADI_ERR* errorCodePtr)
{
    GADI_S32 iavFd;
    GADI_ERR retVal = GADI_OK;
    GADI_VOUT_HandleT* handlePtr = NULL;

    if((errorCodePtr == NULL) || (openParams == NULL))
    {
        *errorCodePtr = GADI_VOUT_ERR_BAD_PARAMETER;
        return NULL;
    }

    if(voutHandle == NULL)
    {
        *errorCodePtr = GADI_VOUT_ERR_NOT_INITIALIZED;
        return NULL;
    }

    handlePtr = voutHandle;

    /*set default value.*/
    retVal = gadi_priv_iav_get_fd(&iavFd);
    if(retVal != GADI_OK)
    {
        GADI_ERROR("gadi_vout_open: gadi_dsp_iav_get_fd: retVal =%d\n", retVal);
        *errorCodePtr = retVal;
        return NULL;
    }

    handlePtr->fdIav        = iavFd;
    handlePtr->resoluMode   = openParams->resoluMode;
    handlePtr->frameRate    = openParams->frameRate;
    handlePtr->devFlag      = openParams->devFlag;
    handlePtr->sinkType     = openParams->sinkType;
    handlePtr->cscEnable    = openParams->cscEnable;
    handlePtr->videoEnable  = openParams->videoEnable;
    handlePtr->rotateEnable = openParams->rotateEnable;
    handlePtr->fbId         = openParams->fbId;
    gadi_sys_memcpy(&(handlePtr->videoSize),
                    &(openParams->videoSize), sizeof(GADI_VOUT_VideoSizeT));
    gadi_sys_memcpy(&(handlePtr->videoOffset),
                    &(openParams->videoOffset), sizeof(GADI_VOUT_VideoOffsetT));

    return (GADI_SYS_HandleT)handlePtr;

}

GADI_ERR gadi_vout_close(GADI_SYS_HandleT handle)
{
    GADI_VOUT_HandleT* voutHandleTemp = (GADI_VOUT_HandleT*)handle;

    if(voutHandleTemp == NULL)
    {
        return GADI_VOUT_ERR_BAD_PARAMETER;
    }

    gadi_sys_memset(voutHandleTemp, 0, sizeof(GADI_VOUT_HandleT));

    return GADI_OK;
}

GADI_ERR gadi_vout_get_params(GADI_SYS_HandleT handle, GADI_VOUT_SettingParamsT* voutParams)
{
    GADI_VOUT_HandleT* voutHandleTemp = (GADI_VOUT_HandleT*)handle;

    if((voutHandleTemp == NULL) || (voutParams == NULL))
    {
        return GADI_VOUT_ERR_BAD_PARAMETER;
    }

    voutParams->resoluMode = voutHandleTemp->resoluMode;
    voutParams->sinkType   = voutHandleTemp->sinkType;

    return GADI_OK;
}

GADI_ERR gadi_vout_set_params(GADI_SYS_HandleT handle, GADI_VOUT_SettingParamsT* voutParams)
{
    GADI_ERR retVal = GADI_OK;
    GADI_VOUT_HandleT* voutHandleTemp = (GADI_VOUT_HandleT*)handle;

    if((voutHandleTemp == NULL) || (voutParams == NULL))
    {
        return GADI_VOUT_ERR_BAD_PARAMETER;
    }

    retVal = vout_setup_params(voutHandleTemp, voutParams);
    if(retVal == GADI_OK)
    {
        voutHandleTemp->resoluMode = voutParams->resoluMode;
        voutHandleTemp->sinkType   = voutParams->sinkType;
    }

    return retVal;
}

//*****************************************************************************
//*****************************************************************************
//** Local Functions
//*****************************************************************************
//*****************************************************************************
static GADI_ERR vout_get_resolution(GADI_U32 mode, GADI_S32 *width, GADI_S32 *height)
{
    GADI_U32 cnt;
    for (cnt = 0; cnt < ARRAY_SIZE(voutRes); ++cnt)
    {
        if (voutRes[cnt].mode == mode)
        {
            *width  = voutRes[cnt].width;
            *height = voutRes[cnt].height;

            return GADI_OK;
        }
    }

    return GADI_VOUT_ERR_BAD_PARAMETER;
}

static GADI_ERR vout_check_params(GADI_VOUT_HandleT* handlePtr, GADI_VOUT_SettingParamsT* params)
{
    GADI_S32 width  = 0;
    GADI_S32 height = 0;
    GADI_S32 blankX = 0;
    GADI_S32 blankY = 0;

    if (vout_get_resolution(params->resoluMode, &width, &height) < 0)
    {
        GADI_ERROR("input invalid vout mode [0x%x]!\n", params->resoluMode);
        return GADI_VOUT_ERR_BAD_PARAMETER;
    }

    /*Check video size*/
    if (handlePtr->videoSize.specified)
    {
        if (handlePtr->videoSize.voutWidth > width)
        {
            handlePtr->videoSize.voutWidth  = width;
            handlePtr->videoSize.videoWidth = width;
            GADI_INFO("Vout video width change to %d\n", width);
        }
        if (handlePtr->videoSize.voutHeight > height)
        {
            handlePtr->videoSize.voutHeight  = height;
            handlePtr->videoSize.videoHeight = height;
            GADI_INFO("Vout video height change to %d\n", height);
        }
    }
    else
    {
        handlePtr->videoSize.voutWidth   = width;
        handlePtr->videoSize.voutHeight  = height;
        handlePtr->videoSize.videoWidth  = width;
        handlePtr->videoSize.videoHeight = height;
    }

    /*Check video offset*/
    blankX = width  - handlePtr->videoSize.videoWidth;
    blankY = height - handlePtr->videoSize.videoHeight;

    if (handlePtr->videoOffset.specified)
    {
        if (handlePtr->videoOffset.offsetX < 0)
        {
            handlePtr->videoOffset.offsetX= 0;
        }
        if (handlePtr->videoOffset.offsetX > blankX)
        {
            handlePtr->videoOffset.offsetX = blankX;
        }
        if (handlePtr->videoOffset.offsetY < 0)
        {
            handlePtr->videoOffset.offsetY = 0;
        }
        if (handlePtr->videoOffset.offsetY > blankY)
        {
            handlePtr->videoOffset.offsetY = blankY;
        }
    }
    else
    {
        handlePtr->videoOffset.offsetX = (blankX >> 1);
        handlePtr->videoOffset.offsetY = (blankY >> 1);
    }

    /*check resource limit.*/
    //__set_resource_limit_if_necessary

    return GADI_OK;
}

static GADI_ERR vout_get_sink_id(GADI_VOUT_HandleT* handlePtr, GADI_U32 sinkType, GADI_U32* sinkId)
{
    GADI_S32 num = 0;
    GADI_S32 cnt;
    struct amba_vout_sink_info sinkInfo;

    if(sinkId == NULL)
    {
        return GADI_VOUT_ERR_BAD_PARAMETER;
    }

    if (ioctl(handlePtr->fdIav, IAV_IOC_VOUT_GET_SINK_NUM, &num) < 0)
    {
        perror("IAV_IOC_VOUT_GET_SINK_NUM\n");
        return GADI_VOUT_ERR_FROM_DRIVER;
    }
    if (num < 1)
    {
        GADI_ERROR("Please load vout driver!\n");
        return GADI_VOUT_ERR_FROM_DRIVER;
    }

    for (cnt = num - 1; cnt >= 0; --cnt)
    {
        sinkInfo.id = cnt;
        if (ioctl(handlePtr->fdIav, IAV_IOC_VOUT_GET_SINK_INFO, &sinkInfo) < 0)
        {
            perror("IAV_IOC_VOUT_GET_SINK_INFO\n");
            return GADI_VOUT_ERR_FROM_DRIVER;
        }

        GADI_INFO("sink %d is %s\n", sinkInfo.id, sinkInfo.name);

        if ((sinkInfo.sink_type == sinkType) && (sinkInfo.source_id == 1))
        {
            *sinkId = sinkInfo.id;
        }
    }

    GADI_INFO("%s: 1 %d, return %d\n", __func__, sinkType, *sinkId);

    return GADI_OK;
}

static GADI_ERR vout_setup_params(GADI_VOUT_HandleT* handlePtr, GADI_VOUT_SettingParamsT* params)
{
    GADI_ERR retVal = GADI_OK;
    GADI_U32 sinkId = 0;
    GADI_U32 sinkType;
    struct amba_video_sink_mode  sinkCfg;
    struct amba_vout_osd_rescale osdRescale;
    struct amba_vout_osd_offset  osdOffset;


    if ((handlePtr == NULL) || (handlePtr->fdIav <= 0) || (params == NULL))
    {
        return GADI_VOUT_ERR_BAD_PARAMETER;
    }

    retVal = vout_check_params(handlePtr, params);
    if(retVal != GADI_OK)
    {
        GADI_ERROR("vout_check_params error\n");
        return retVal;
    }

    /*Specify external device type for vout*/
    if (handlePtr->devFlag) {
        sinkType = params->sinkType;
        retVal = vout_get_sink_id(handlePtr, sinkType, &sinkId);
        if(retVal != GADI_OK)
        {
            GADI_ERROR("vout_get_sink_id error\n");
            return retVal;
        }

        if (ioctl(handlePtr->fdIav, IAV_IOC_VOUT_SELECT_DEV, sinkId) < 0)
        {
            perror("IAV_IOC_VOUT_SELECT_DEV");

            switch (sinkType)
            {
                case AMBA_VOUT_SINK_TYPE_CVBS:
                    GADI_INFO("No CVBS sink: Driver not loaded!\n");
                    break;
                case AMBA_VOUT_SINK_TYPE_YPBPR:
                    GADI_INFO("No YPBPR sink: Driver not loaded or ypbpr output not supported!\n");
                    break;
                case AMBA_VOUT_SINK_TYPE_DIGITAL:
                    GADI_INFO("No DIGITAL sink: Driver not loaded or digital output not supported!\n");
                    break;
                case AMBA_VOUT_SINK_TYPE_HDMI:
                    GADI_INFO("No HDMI sink: Hdmi cable not plugged, driver not loaded, or hdmi output not supported\n");
                    break;
                default:
                    break;
            }
            return GADI_VOUT_ERR_BAD_PARAMETER;
        }
    }

    /*configure vout*/
    memset(&sinkCfg, 0, sizeof(sinkCfg));
    memset(&osdRescale, 0, sizeof(osdRescale));
    memset(&osdOffset, 0, sizeof(osdOffset));

    if (params->sinkType == AMBA_VOUT_SINK_TYPE_DIGITAL)
    {
        /*set LCD digital as default*/
        sinkCfg.type      = AMBA_VIDEO_TYPE_AUTO;
        sinkCfg.sink_type = AMBA_VOUT_SINK_TYPE_DIGITAL;

    }
    else
    {
        sinkCfg.type      = params->sinkType;
        sinkCfg.sink_type = AMBA_VOUT_SINK_TYPE_AUTO;
    }
    sinkCfg.id                = sinkId;
    sinkCfg.mode              = params->resoluMode;
    sinkCfg.ratio             = AMBA_VIDEO_RATIO_AUTO;
    sinkCfg.bits              = AMBA_VIDEO_BITS_AUTO;
    sinkCfg.format            = AMBA_VIDEO_FORMAT_AUTO;
    sinkCfg.frame_rate        = handlePtr->frameRate;
    sinkCfg.csc_en            = handlePtr->cscEnable;
    sinkCfg.hdmi_3d_structure = DDD_RESERVED;
    sinkCfg.video_en          = handlePtr->videoEnable;
    if (handlePtr->rotateEnable)
    {
        sinkCfg.video_rotate  = AMBA_VOUT_ROTATE_90;
    }
    else
    {
        sinkCfg.video_rotate  = AMBA_VOUT_ROTATE_NORMAL;
    }
    gadi_sys_memcpy(&(sinkCfg.video_size), &(handlePtr->videoSize), sizeof(GADI_VOUT_VideoSizeT));
    gadi_sys_memcpy(&(sinkCfg.video_offset), &(handlePtr->videoOffset), sizeof(GADI_VOUT_VideoOffsetT));

    sinkCfg.fb_id             = -2;
    sinkCfg.osd_rescale       = osdRescale;
    sinkCfg.osd_offset        = osdOffset;
    sinkCfg.bg_color.y        = 0x10;
    sinkCfg.bg_color.cb       = 0x80;
    sinkCfg.bg_color.cr       = 0x80;
    sinkCfg.lcd_cfg.mode      = AMBA_VOUT_LCD_MODE_DISABLE;

    if (ioctl(handlePtr->fdIav, IAV_IOC_VOUT_CONFIGURE_SINK, &sinkCfg) < 0)
    {
        perror("IAV_IOC_VOUT_CONFIGURE_SINK\n");
        return GADI_VOUT_ERR_FROM_DRIVER;
    }

    return GADI_OK;
}


