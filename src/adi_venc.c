/*!
*****************************************************************************
** \file        adi/src/adi_venc.c
**
** \version     $Id: adi_venc.c 1 2014-08-11 09:24:58Z fengxuequan $
**
** \brief       ADI video encode module function
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

#include "adi_types.h"
#include "adi_sys.h"
#include "adi_priv.h"
#include "adi_venc.h"


//*****************************************************************************
//*****************************************************************************
//** Local Defines
//*****************************************************************************
//*****************************************************************************
#define DEFAULT_ENCODE_MODE GADI_VENC_MODE_NORMAL
#define STREAM_MASK        (0xF)

#define MAX_ZOOM_IN_MAIN_BUFFER     (1100)      // 11X
#define MAX_ZOOM_IN_PREVIEW_A_B     (500)       // 5X
#define MAX_ZOOM_STEPS              (10)


//*****************************************************************************
//*****************************************************************************
//** Local structures
//*****************************************************************************
//*****************************************************************************
typedef struct
{
    /*file descriptor of IAV device.*/
    GADI_S32                fdIav;
    /*video input module handle.*/
    GADI_SYS_HandleT        vinHandle;
    /*video output module handle.*/
    GADI_SYS_HandleT        voutHandle;
    /*encode mode, enum:GADI_VENC_ModeEnumT*/
    GADI_U32                encodeMode;
    /*active encoding stream number.*/
    GADI_U32                activeEncodingNum;
    /*encode streams parameters.*/
    GADI_VENC_StreamParamsT encodeStreams[GADI_VENC_STREAM_NUM];
    /*source buffer parameters.*/
    GADI_VENC_BufferParamsT sourceBuffers[GADI_VENC_BUFFER_NUM];
}GADI_VENC_HandleT;


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
static GADI_VENC_HandleT* vencHandle;


//*****************************************************************************
//*****************************************************************************
//** Local Functions Declaration
//*****************************************************************************
//*****************************************************************************
static GADI_U32 venc_stream_count(GADI_U32 idFormat);
static GADI_ERR venc_set_encode_fps(GADI_VENC_HandleT* handlePtr,
                                           GADI_U32 idFormat,
                                           GADI_U32 fps,
                                           GADI_U32 vinFrameRate);
static GADI_ERR venc_get_dptz_factor(GADI_U32 *newFactor,
                                           GADI_U32 oldFactor,
                                           GADI_U32 bufId,
                                           GADI_VENC_HandleT *vencHandle);
static GADI_ERR venc_calc_dptz_I_param(GADI_VENC_DptzParamT *vencDptz,
                                               iav_digital_zoom_ex_t *dptz,
                                               GADI_VENC_HandleT *vencHandle);
static GADI_ERR venc_calc_dptz_II_param(GADI_VENC_DptzParamT *vencDptz,
                                          iav_digital_zoom2_ex_t *dptz,
                                          GADI_VENC_HandleT *vencHandle);
static GADI_ERR venc_set_dptz_control_param(GADI_VENC_HandleT *handle,
                                                  GADI_VENC_DptzParamT *vencDptz);
static GADI_ERR venc_zoom_to_max_fov_keep_as(GADI_VENC_HandleT *handle,
                                                              GADI_U32 idFormat);
static GADI_ERR venc_check_resource(GADI_VENC_HandleT *venHandle);
static GADI_ERR venc_set_preview_buf(GADI_VENC_HandleT *venHandle);

//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************
GADI_ERR gadi_venc_init(void)
{
    if(vencHandle != NULL)
    {
        return GADI_VENC_ERR_ALREADY_INITIALIZED;
    }
    vencHandle = gadi_sys_malloc(sizeof(GADI_VENC_HandleT));
    if(vencHandle == NULL)
    {
        GADI_ERROR("gadi_vout_init: malloc memory failed!\n");
        return GADI_VENC_ERR_OUT_OF_MEMORY;
    }

    gadi_sys_memset(vencHandle, 0, sizeof(GADI_VENC_HandleT));


    GADI_INFO(" [DONE] Initialize VENC\n");

    return GADI_OK;
}

GADI_ERR gadi_venc_exit(void)
{
    if(vencHandle == NULL)
    {
        return GADI_VENC_ERR_NOT_INITIALIZED;
    }

    gadi_sys_free(vencHandle);
    vencHandle = NULL;

    return GADI_OK;
}

GADI_SYS_HandleT gadi_venc_open(GADI_VENC_OpenParamsT* openParams, GADI_ERR* errorCodePtr)
{
    GADI_U8 cnt = 0;
    GADI_S32 iavFd;
    GADI_ERR retVal = GADI_OK;
    GADI_VENC_HandleT* handlePtr = NULL;

    if((errorCodePtr == NULL) || (openParams == NULL))
    {
        *errorCodePtr = GADI_VENC_ERR_BAD_PARAMETER;
        return NULL;
    }

    if(openParams->vinHandle == NULL)
    {
        GADI_ERROR("gadi_venc_open: open vin module first.\n");
        *errorCodePtr = GADI_VENC_ERR_BAD_PARAMETER;
        return NULL;
    }

    if(openParams->voutHandle == NULL)
    {
        GADI_ERROR("gadi_venc_open: open vout module first.\n");
        *errorCodePtr = GADI_VENC_ERR_BAD_PARAMETER;
        return NULL;
    }

    if(vencHandle == NULL)
    {
        *errorCodePtr = GADI_VENC_ERR_NOT_INITIALIZED;
        return NULL;
    }

    handlePtr = vencHandle;

    /*set default value.*/
    retVal = gadi_priv_iav_get_fd(&iavFd);
    if(retVal != GADI_OK)
    {
        GADI_ERROR("gadi_vout_open: gadi_dsp_iav_get_fd: retVal =%d\n", retVal);
        *errorCodePtr = retVal;
        return NULL;
    }

    handlePtr->fdIav      = iavFd;
    handlePtr->vinHandle  = openParams->vinHandle;
    handlePtr->voutHandle = openParams->voutHandle;
    handlePtr->encodeMode = DEFAULT_ENCODE_MODE;

    /*set source buffer default information value.*/
    for(cnt = 0; cnt < GADI_VENC_BUFFER_NUM; cnt++)
    {
        /*source buffer id&state.*/
        handlePtr->sourceBuffers[cnt].info.id = 1 << openParams->defBufState[cnt].bufferId;
        handlePtr->sourceBuffers[cnt].info.state = openParams->defBufState[cnt].state;
        /*setup source buffer format.*/
        handlePtr->sourceBuffers[cnt].format.id = 1 << openParams->defBufFormat[cnt].bufferId;
        handlePtr->sourceBuffers[cnt].format.width  = openParams->defBufFormat[cnt].width;
        handlePtr->sourceBuffers[cnt].format.height = openParams->defBufFormat[cnt].height;
        handlePtr->sourceBuffers[cnt].format.deintlc_for_intlc_vin =
                                openParams->defBufFormat[cnt].deintlcForIntlcVin;
        /*source buffer type.*/
        handlePtr->sourceBuffers[cnt].type = openParams->defBufType[cnt].bufferType;
    }

    /*set stream default information value.*/
    for(cnt = 0; cnt < GADI_VENC_STREAM_NUM; cnt++)
    {
        /*stream id&state.*/
        handlePtr->encodeStreams[cnt].info.id = 1 << openParams->defStrState[cnt].streamId;
        handlePtr->encodeStreams[cnt].info.state = openParams->defStrState[cnt].state;
        /*encode format config for each stream.*/
        handlePtr->encodeStreams[cnt].format.id = 1 << openParams->defStrFormat[cnt].streamId;
        handlePtr->encodeStreams[cnt].format.encode_type =
                                openParams->defStrFormat[cnt].encodeType;
        handlePtr->encodeStreams[cnt].format.source =
                                openParams->defStrFormat[cnt].sourceBuf;
        handlePtr->encodeStreams[cnt].format.encode_width =
                                openParams->defStrFormat[cnt].width;
        handlePtr->encodeStreams[cnt].format.encode_height =
                                openParams->defStrFormat[cnt].height;
        handlePtr->encodeStreams[cnt].format.encode_x =
                                openParams->defStrFormat[cnt].xOffset;
        handlePtr->encodeStreams[cnt].format.encode_y =
                                openParams->defStrFormat[cnt].yOffset;
        /*h264 stream config.*/
        handlePtr->encodeStreams[cnt].h264.id =
                                1 << openParams->defH264Config[cnt].streamId;
        handlePtr->encodeStreams[cnt].h264.M =
                                openParams->defH264Config[cnt].gopM;
        handlePtr->encodeStreams[cnt].h264.N =
                                openParams->defH264Config[cnt].gopN;
        handlePtr->encodeStreams[cnt].h264.idr_interval =
                                openParams->defH264Config[cnt].idrInterval;
        handlePtr->encodeStreams[cnt].h264.gop_model =
                                openParams->defH264Config[cnt].gopModel;
        if (openParams->defH264Config[cnt].brcMode & 0x1)
        {
            handlePtr->encodeStreams[cnt].h264.average_bitrate =
                                openParams->defH264Config[cnt].vbrMaxbps;
        }
        else
        {
            handlePtr->encodeStreams[cnt].h264.average_bitrate =
                                openParams->defH264Config[cnt].cbrAvgBps;
        }
        /*mjpeg stream config.*/
        handlePtr->encodeStreams[cnt].mjpeg.id =
                                1 << openParams->defMjpegConfig[cnt].streamId;
        handlePtr->encodeStreams[cnt].mjpeg.chroma_format =
                                openParams->defMjpegConfig[cnt].chromaFormat;
        handlePtr->encodeStreams[cnt].mjpeg.quality =
                                openParams->defMjpegConfig[cnt].quality;
    }

    return (GADI_SYS_HandleT)handlePtr;

}

GADI_ERR gadi_venc_close(GADI_SYS_HandleT handle)
{
    GADI_VENC_HandleT* vencHandleTemp = (GADI_VENC_HandleT*)handle;

    if(vencHandleTemp == NULL)
    {
        return GADI_VENC_ERR_BAD_PARAMETER;
    }

    gadi_sys_memset(vencHandleTemp, 0, sizeof(GADI_VENC_HandleT));

    return GADI_OK;
}

GADI_ERR gadi_venc_get_encode_mode(GADI_SYS_HandleT handle, GADI_U32* encodeMode)
{
    GADI_VENC_HandleT* vencHandleTemp = (GADI_VENC_HandleT*)handle;

    if((vencHandleTemp == NULL) || (encodeMode == NULL))
    {
        return GADI_VENC_ERR_BAD_PARAMETER;
    }

    *encodeMode = vencHandleTemp->encodeMode;

    return GADI_OK;
}

GADI_ERR gadi_venc_set_encode_mode(GADI_SYS_HandleT handle, GADI_U32 encodeMode)
{
    GADI_VENC_HandleT* vencHandleTemp = (GADI_VENC_HandleT*)handle;

    if((vencHandleTemp == NULL) || (vencHandleTemp->fdIav <= 0))
    {
        return GADI_VENC_ERR_BAD_PARAMETER;
    }

    vencHandleTemp->encodeMode = encodeMode;

    return GADI_OK;
}

GADI_ERR gadi_venc_start_stream(GADI_SYS_HandleT handle, GADI_U32 streamId)
{
    GADI_U32 i     = 0;
    GADI_U32 streamCount = 0;
    GADI_U32 indexFormat = 0;
    iav_encode_stream_info_ex_t info;
    GADI_VENC_HandleT* vencHandleTemp = (GADI_VENC_HandleT*)handle;

    if((vencHandleTemp == NULL) || (vencHandleTemp->fdIav <= 0))
    {
        return GADI_VENC_ERR_BAD_PARAMETER;
    }

    indexFormat = 1 << streamId;
    for (i = 0; i < GADI_VENC_STREAM_NUM; ++i)
    {
        info.id = (1 << i);
        if (ioctl(vencHandleTemp->fdIav, IAV_IOC_GET_ENCODE_STREAM_INFO_EX, &info) < 0)
        {
            perror("IAV_IOC_GET_ENCODE_STREAM_INFO_EX");
            continue;
        }
        if (info.state == IAV_STREAM_STATE_ENCODING)
        {
            indexFormat &= ~(1 << i);
        }
    }
    if (indexFormat == 0)
    {
        return 0;
    }

    if (vencHandleTemp->encodeMode != GADI_VENC_MODE_LOW_DELAY)
    {
        venc_zoom_to_max_fov_keep_as(vencHandleTemp, indexFormat);
    }

    if (ioctl(vencHandleTemp->fdIav, IAV_IOC_START_ENCODE_EX, indexFormat) < 0)
    {
        perror("IAV_IOC_START_ENCODE_EX");
        return GADI_VENC_ERR_FROM_DRIVER;
    }

    streamCount = venc_stream_count(indexFormat);
    vencHandleTemp->activeEncodingNum += streamCount;

    GADI_INFO("Start encoding for stream 0x%x.\t\tActive encoding num is %d.\n",
               streamId, vencHandleTemp->activeEncodingNum);

    return GADI_OK;
}

GADI_ERR gadi_venc_stop_stream(GADI_SYS_HandleT handle, GADI_U32 streamId)
{
    GADI_U32 i           = 0;
    GADI_U32 streamCount = 0;
    GADI_U32 indexFormat = 0;
    GADI_U32 iavState    = 0;
    iav_encode_stream_info_ex_t streamInfo;
    GADI_VENC_HandleT* vencHandleTemp = (GADI_VENC_HandleT*)handle;

    if((vencHandleTemp == NULL) || (vencHandleTemp->fdIav <= 0))
    {
        return GADI_VENC_ERR_BAD_PARAMETER;
    }

    indexFormat = 1 << streamId;

    if (ioctl(vencHandleTemp->fdIav, IAV_IOC_GET_STATE, &iavState) < 0)
    {
        perror("IAV_IOC_GET_STATE\n");
        return GADI_VENC_ERR_FROM_DRIVER;
    }
    if (iavState != IAV_STATE_ENCODING)
    {
        GADI_INFO("gadi_venc_stop_stream: driver state:%d\n",iavState);
        return GADI_OK;
    }

    for (i = 0; i < GADI_VENC_STREAM_NUM; ++i)
    {
        streamInfo.id = (1 << i);
        if (ioctl(vencHandleTemp->fdIav, IAV_IOC_GET_ENCODE_STREAM_INFO_EX, &streamInfo) < 0)
        {
            perror("IAV_IOC_GET_ENCODE_STREAM_INFO_EX\n");
            continue;
        }
        if (streamInfo.state != IAV_STREAM_STATE_ENCODING)
        {
            indexFormat &= ~(1 << i);
        }
    }
    if (indexFormat == 0)
    {
        GADI_INFO("gadi_venc_stop_stream: stream:%d is not encoding\n",streamId);
        return GADI_OK;
    }

    streamCount = venc_stream_count(indexFormat);
    if (ioctl(vencHandleTemp->fdIav, IAV_IOC_STOP_ENCODE_EX, indexFormat) < 0)
    {
        perror("IAV_IOC_STOP_ENCODE_EX\n");
        return GADI_VENC_ERR_FROM_DRIVER;
    }
    vencHandleTemp->activeEncodingNum -= streamCount;
    GADI_INFO("Stop encoding for stream 0x%x.\t\tAcitve encoding num is %d.\n",
              streamId, vencHandleTemp->activeEncodingNum);

    return GADI_OK;
}

GADI_ERR gadi_venc_force_idr(GADI_SYS_HandleT handle, GADI_U32 streamId)
{
    GADI_U32 indexFormat = 0;
    GADI_VENC_HandleT* vencHandleTemp = (GADI_VENC_HandleT*)handle;

    if((vencHandleTemp == NULL) || (vencHandleTemp->fdIav <= 0))
    {
        return GADI_VENC_ERR_BAD_PARAMETER;
    }

    indexFormat = 1 << streamId;

    if (ioctl(vencHandleTemp->fdIav, IAV_IOC_FORCE_IDR_EX, indexFormat) < 0)
    {
        perror("IAV_IOC_FORCE_IDR_EX\n");
        GADI_ERROR("gadi_venc_force_idr:Force IDR for stream %d Failed!\n", streamId);
        return GADI_VENC_ERR_FROM_DRIVER;
    }

    GADI_INFO("Force IDR for stream %d OK!\n", streamId);

    return GADI_OK;
}

GADI_ERR gadi_venc_set_bitrate(GADI_SYS_HandleT handle, GADI_VENC_BitRateRangeT* brPar)
{
    iav_bitrate_info_ex_t brInfo;

    GADI_VENC_HandleT* vencHandleTemp = (GADI_VENC_HandleT*)handle;

    if((vencHandleTemp == NULL) || (vencHandleTemp->fdIav <= 0) || (brPar == NULL))
    {
        return GADI_VENC_ERR_BAD_PARAMETER;
    }

    brInfo.id = 1 << brPar->streamId;
    if (ioctl(vencHandleTemp->fdIav, IAV_IOC_GET_BITRATE_EX, &brInfo) < 0)
    {
        perror("IAV_IOC_GET_BITRATE_EX\n");
        return GADI_VENC_ERR_FROM_DRIVER;
    }
    if (brInfo.rate_control_mode & 0x1)
    {
        brInfo.vbr_min_bitrate = brPar->minBps;
        brInfo.vbr_max_bitrate = brPar->maxBps;
    }
    else
    {
        brInfo.cbr_avg_bitrate = brPar->minBps;
    }
    if (ioctl(vencHandleTemp->fdIav, IAV_IOC_SET_BITRATE_EX, &brInfo) < 0)
    {
        perror("IAV_IOC_SET_BITRATE_EX\n");
        GADI_ERROR("gadi_venc_set_bitrate: stream %d, change bitrate min [%d], max [%d].\n",
                brPar->streamId, brPar->minBps, brPar->maxBps);
        return GADI_VENC_ERR_FROM_DRIVER;
    }

    GADI_INFO("stream %d Change bitrate range to [%d, %d] successful!\n",
              brPar->streamId, brPar->minBps, brPar->maxBps);

    return GADI_OK;
}

GADI_ERR gadi_venc_set_framerate(GADI_SYS_HandleT handle, GADI_VENC_FrameRateT* frPar)
{
    GADI_ERR retVal;
    GADI_VENC_HandleT *vencHandleTemp = (GADI_VENC_HandleT*)handle;
    GADI_VIN_HandleT *vinHandleTemp = NULL;

    if((vencHandleTemp == NULL) || (vencHandleTemp->fdIav <= 0) || (frPar == NULL))
    {
        return GADI_VENC_ERR_BAD_PARAMETER;
    }

    vinHandleTemp = (GADI_VIN_HandleT*)vencHandleTemp->vinHandle;

    retVal = venc_set_encode_fps(vencHandleTemp,
                                 (1 << frPar->streamId),
                                 frPar->fps,
                                 vinHandleTemp->frameRate);

    return retVal;
}

GADI_ERR gadi_venc_get_buffer_state(GADI_SYS_HandleT handle, GADI_VENC_BufferStateT* statePar)
{
    iav_source_buffer_info_ex_t*  bufInfoPar = NULL;
    GADI_VENC_HandleT* vencHandleTemp = (GADI_VENC_HandleT*)handle;

    if((vencHandleTemp == NULL) || (vencHandleTemp->fdIav <= 0) || (statePar == NULL))
    {
        return GADI_VENC_ERR_BAD_PARAMETER;
    }

    bufInfoPar = &(vencHandleTemp->sourceBuffers[statePar->bufferId].info);

    bufInfoPar->id = 1 << statePar->bufferId;

    if (ioctl(vencHandleTemp->fdIav, IAV_IOC_GET_SOURCE_BUFFER_INFO_EX, bufInfoPar) < 0)
    {
        perror("IAV_IOC_GET_SOURCE_BUFFER_INFO_EX\n");
        return GADI_VENC_ERR_FROM_DRIVER;
    }

    statePar->state = bufInfoPar->state;

    return GADI_OK;
}

GADI_ERR gadi_venc_get_buffer_format_all(GADI_SYS_HandleT handle,
                                        GADI_VENC_BufferFormatAllT* formatAllPar)
{
    GADI_U8 cnt = 0;
    iav_source_buffer_format_ex_t* bufFormatPar = NULL;
    GADI_VENC_HandleT* vencHandleTemp = (GADI_VENC_HandleT*)handle;

    if((vencHandleTemp == NULL) || (vencHandleTemp->fdIav <= 0) || (formatAllPar == NULL))
    {
        return GADI_VENC_ERR_BAD_PARAMETER;
    }

    for(cnt=0; cnt < GADI_VENC_BUFFER_NUM; cnt++)
    {
        bufFormatPar = &(vencHandleTemp->sourceBuffers[cnt].format);

        bufFormatPar->id = 1 << cnt;
        if (ioctl(vencHandleTemp->fdIav, IAV_IOC_GET_SOURCE_BUFFER_FORMAT_EX, bufFormatPar) < 0)
        {
            perror("IAV_IOC_GET_SOURCE_BUFFER_FORMAT_EX\n");
            return GADI_VENC_ERR_FROM_DRIVER;
        }
    }

    formatAllPar->mainWidth  = vencHandleTemp->sourceBuffers[GADI_VENC_BUFFER_MAIN].format.width;
    formatAllPar->mainHeight = vencHandleTemp->sourceBuffers[GADI_VENC_BUFFER_MAIN].format.height;
    formatAllPar->mainDeintlcForIntlcVin =
        vencHandleTemp->sourceBuffers[GADI_VENC_BUFFER_MAIN].format.deintlc_for_intlc_vin;

    formatAllPar->secondWidth  = vencHandleTemp->sourceBuffers[GADI_VENC_BUFFER_SECOND].format.width;
    formatAllPar->secondHeight = vencHandleTemp->sourceBuffers[GADI_VENC_BUFFER_SECOND].format.height;
    formatAllPar->secondDeintlcForIntlcVin =
        vencHandleTemp->sourceBuffers[GADI_VENC_BUFFER_SECOND].format.deintlc_for_intlc_vin;

    formatAllPar->thirdWidth  = vencHandleTemp->sourceBuffers[GADI_VENC_BUFFER_THIRD].format.width;
    formatAllPar->thirdHeight = vencHandleTemp->sourceBuffers[GADI_VENC_BUFFER_THIRD].format.height;
    formatAllPar->thirdDeintlcForIntlcVin =
        vencHandleTemp->sourceBuffers[GADI_VENC_BUFFER_THIRD].format.deintlc_for_intlc_vin;

    formatAllPar->fourthWidth  = vencHandleTemp->sourceBuffers[GADI_VENC_BUFFER_FOURTH].format.width;
    formatAllPar->fourthHeight = vencHandleTemp->sourceBuffers[GADI_VENC_BUFFER_FOURTH].format.height;
    formatAllPar->fourthDeintlcForIntlcVin =
        vencHandleTemp->sourceBuffers[GADI_VENC_BUFFER_FOURTH].format.deintlc_for_intlc_vin;

    return GADI_OK;
}

GADI_ERR gadi_venc_set_buffer_format_all(GADI_SYS_HandleT handle,
                                       GADI_VENC_BufferFormatAllT* formatAllPar)
{
    GADI_ERR retVal;
    iav_source_buffer_format_all_ex_t bufFormatAll;
    GADI_VENC_HandleT* vencHandleTemp = (GADI_VENC_HandleT*)handle;

    if((vencHandleTemp == NULL) || (vencHandleTemp->fdIav <= 0) || (formatAllPar == NULL))
    {
        return GADI_VENC_ERR_BAD_PARAMETER;
    }

    bufFormatAll.main_width  = formatAllPar->mainWidth;
    bufFormatAll.main_height = formatAllPar->mainHeight;
    bufFormatAll.main_deintlc_for_intlc_vin = formatAllPar->mainDeintlcForIntlcVin;

    bufFormatAll.second_width  = formatAllPar->secondWidth;
    bufFormatAll.second_height = formatAllPar->secondHeight;
    bufFormatAll.second_deintlc_for_intlc_vin = formatAllPar->secondDeintlcForIntlcVin;

    bufFormatAll.third_width  = formatAllPar->thirdWidth;
    bufFormatAll.third_height = formatAllPar->thirdHeight;
    bufFormatAll.third_deintlc_for_intlc_vin = formatAllPar->thirdDeintlcForIntlcVin;

    bufFormatAll.fourth_width  = formatAllPar->fourthWidth;
    bufFormatAll.fourth_height = formatAllPar->fourthHeight;
    bufFormatAll.fourth_deintlc_for_intlc_vin = formatAllPar->fourthDeintlcForIntlcVin;

    bufFormatAll.intlc_scan = formatAllPar->intlcScan;


    bufFormatAll.second_input_width  = bufFormatAll.main_width;
    bufFormatAll.second_input_height = bufFormatAll.main_height;
    bufFormatAll.third_input_width   = bufFormatAll.main_width;
    bufFormatAll.third_input_height  = bufFormatAll.main_height;
    bufFormatAll.fourth_input_width  = bufFormatAll.main_width;
    bufFormatAll.fourth_input_height = bufFormatAll.main_height;

    /*set handle's parameters*/
    vencHandleTemp->sourceBuffers[GADI_VENC_BUFFER_MAIN].format.width  = formatAllPar->mainWidth;
    vencHandleTemp->sourceBuffers[GADI_VENC_BUFFER_MAIN].format.height = formatAllPar->mainHeight;
    vencHandleTemp->sourceBuffers[GADI_VENC_BUFFER_MAIN].format.deintlc_for_intlc_vin =
                                           formatAllPar->mainDeintlcForIntlcVin;

    vencHandleTemp->sourceBuffers[GADI_VENC_BUFFER_SECOND].format.width  = formatAllPar->secondWidth;
    vencHandleTemp->sourceBuffers[GADI_VENC_BUFFER_SECOND].format.height = formatAllPar->secondHeight;
    vencHandleTemp->sourceBuffers[GADI_VENC_BUFFER_SECOND].format.deintlc_for_intlc_vin =
                                           formatAllPar->secondDeintlcForIntlcVin;

    vencHandleTemp->sourceBuffers[GADI_VENC_BUFFER_THIRD].format.width  = formatAllPar->thirdWidth;
    vencHandleTemp->sourceBuffers[GADI_VENC_BUFFER_THIRD].format.height = formatAllPar->thirdHeight;
    vencHandleTemp->sourceBuffers[GADI_VENC_BUFFER_THIRD].format.deintlc_for_intlc_vin =
                                            formatAllPar->thirdDeintlcForIntlcVin;

    vencHandleTemp->sourceBuffers[GADI_VENC_BUFFER_FOURTH].format.width  = formatAllPar->fourthWidth;
    vencHandleTemp->sourceBuffers[GADI_VENC_BUFFER_FOURTH].format.height = formatAllPar->fourthHeight;
    vencHandleTemp->sourceBuffers[GADI_VENC_BUFFER_FOURTH].format.deintlc_for_intlc_vin =
                                            formatAllPar->fourthDeintlcForIntlcVin;

    /*check resource limit*/
    retVal = venc_check_resource(vencHandleTemp);


    if (ioctl(vencHandleTemp->fdIav, IAV_IOC_SET_SOURCE_BUFFER_FORMAT_ALL_EX, &bufFormatAll) < 0)
    {
        perror("IAV_IOC_SET_SOURCE_BUFFER_FORMAT_ALL_EX\n");
        return GADI_VENC_ERR_FROM_DRIVER;
    }

    return retVal;
}


GADI_ERR gadi_venc_get_buffer_type_all(GADI_SYS_HandleT handle,
                                            GADI_VENC_BufferTypeAllT* typeAllPar)
{
    iav_source_buffer_type_all_ex_t bufTypeAll;
    GADI_VENC_HandleT* vencHandleTemp = (GADI_VENC_HandleT*)handle;

    if((vencHandleTemp == NULL) || (vencHandleTemp->fdIav <= 0) || (typeAllPar == NULL))
    {
        return GADI_VENC_ERR_BAD_PARAMETER;
    }

    if (ioctl(vencHandleTemp->fdIav, IAV_IOC_GET_SOURCE_BUFFER_TYPE_ALL_EX, &bufTypeAll) < 0)
    {
        perror("IAV_IOC_GET_SOURCE_BUFFER_TYPE_ALL_EX\n");
        return GADI_VENC_ERR_FROM_DRIVER;
    }

    typeAllPar->mainBufType   = bufTypeAll.main_buffer_type;
    typeAllPar->secondBufType = bufTypeAll.second_buffer_type;
    typeAllPar->thirdBufType  = bufTypeAll.third_buffer_type;
    typeAllPar->fourthBufType = bufTypeAll.fourth_buffer_type;

    /*set handle's parameters*/
    vencHandleTemp->sourceBuffers[GADI_VENC_BUFFER_MAIN].type    = bufTypeAll.main_buffer_type;
    vencHandleTemp->sourceBuffers[GADI_VENC_BUFFER_SECOND].type  = bufTypeAll.second_buffer_type;
    vencHandleTemp->sourceBuffers[GADI_VENC_BUFFER_THIRD].type   = bufTypeAll.third_buffer_type;
    vencHandleTemp->sourceBuffers[GADI_VENC_BUFFER_FOURTH].type  = bufTypeAll.fourth_buffer_type;

    return GADI_OK;
}

GADI_ERR gadi_venc_set_buffer_type_all(GADI_SYS_HandleT handle,
                                           GADI_VENC_BufferTypeAllT* typeAllPar)
{
    GADI_ERR retVal;
    iav_source_buffer_type_all_ex_t bufTypeAll;
    GADI_VENC_HandleT* vencHandleTemp = (GADI_VENC_HandleT*)handle;

    if((vencHandleTemp == NULL) || (vencHandleTemp->fdIav <= 0) || (typeAllPar == NULL))
    {
        return GADI_VENC_ERR_BAD_PARAMETER;
    }

    bufTypeAll.main_buffer_type   = typeAllPar->mainBufType;
    bufTypeAll.second_buffer_type = typeAllPar->secondBufType;
    bufTypeAll.third_buffer_type  = typeAllPar->thirdBufType;
    bufTypeAll.fourth_buffer_type = typeAllPar->fourthBufType;

    if (ioctl(vencHandleTemp->fdIav, IAV_IOC_SET_SOURCE_BUFFER_TYPE_ALL_EX, &bufTypeAll) < 0)
    {
        perror("IAV_IOC_SET_SOURCE_BUFFER_TYPE_ALL_EX\n");
        return GADI_VENC_ERR_FROM_DRIVER;
    }

    /*set handle's parameters*/
    vencHandleTemp->sourceBuffers[GADI_VENC_BUFFER_MAIN].type    = typeAllPar->mainBufType;
    vencHandleTemp->sourceBuffers[GADI_VENC_BUFFER_SECOND].type  = typeAllPar->secondBufType;
    vencHandleTemp->sourceBuffers[GADI_VENC_BUFFER_THIRD].type   = typeAllPar->thirdBufType;
    vencHandleTemp->sourceBuffers[GADI_VENC_BUFFER_FOURTH].type  = typeAllPar->fourthBufType;

    /*setup preview(video output) buffer format.*/
    retVal = venc_set_preview_buf(vencHandleTemp);

    return retVal;
}

GADI_ERR gadi_venc_get_stream_state(GADI_SYS_HandleT handle,
                                               GADI_VENC_StreamStateT* statePar)
{
    iav_encode_stream_info_ex_t* strInfo;
    GADI_VENC_HandleT* vencHandleTemp = (GADI_VENC_HandleT*)handle;

    if((vencHandleTemp == NULL) || (vencHandleTemp->fdIav <= 0) || (statePar == NULL))
    {
        return GADI_VENC_ERR_BAD_PARAMETER;
    }

    strInfo = &(vencHandleTemp->encodeStreams[statePar->streamId].info);

    strInfo->id = 1 << statePar->streamId;
    if (ioctl(vencHandleTemp->fdIav, IAV_IOC_GET_ENCODE_STREAM_INFO_EX, strInfo) < 0)
    {
        perror("IAV_IOC_GET_ENCODE_STREAM_INFO_EX\n");
        return GADI_VENC_ERR_FROM_DRIVER;
    }
    statePar->state = strInfo->state;

    return GADI_OK;
}

GADI_ERR gadi_venc_get_stream_format(GADI_SYS_HandleT handle,
                                             GADI_VENC_StreamFormatT *formatPar)
{
    iav_encode_format_ex_t *strFormat;
    iav_h264_config_ex_t   *h264Config;
    GADI_VENC_HandleT *vencHandleTemp = (GADI_VENC_HandleT*)handle;
    GADI_VIN_HandleT  *vinHandleTemp = NULL;

    if((vencHandleTemp == NULL) || (vencHandleTemp->fdIav <= 0) || (formatPar == NULL))
    {
        return GADI_VENC_ERR_BAD_PARAMETER;
    }

    strFormat = &(vencHandleTemp->encodeStreams[formatPar->streamId].format);

    strFormat->id = 1 << formatPar->streamId;

    if (ioctl(vencHandleTemp->fdIav, IAV_IOC_GET_ENCODE_FORMAT_EX, strFormat) < 0)
    {
        perror("IAV_IOC_GET_ENCODE_FORMAT_EX\n");
        return GADI_VENC_ERR_FROM_DRIVER;
    }

    vinHandleTemp = (GADI_VIN_HandleT*)vencHandleTemp->vinHandle;

    formatPar->encodeType = strFormat->encode_type;
    formatPar->sourceBuf  = strFormat->source;
    formatPar->width      = strFormat->encode_width;
    formatPar->height     = strFormat->encode_height;
    formatPar->xOffset    = strFormat->encode_x;
    formatPar->yOffset    = strFormat->encode_y;

    switch (vinHandleTemp->frameRate)
    {
        case AMBA_VIDEO_FPS_29_97:
            formatPar->fps = AMBA_VIDEO_FPS_30;
            break;
        case AMBA_VIDEO_FPS_59_94:
            formatPar->fps = AMBA_VIDEO_FPS_60;
            break;
        default:
            formatPar->fps = vinHandleTemp->frameRate;
            break;
    }

    h264Config = &(vencHandleTemp->encodeStreams[formatPar->streamId].h264);

    formatPar->flipRotate = ((h264Config->rotate_clockwise << 2) |
                             (h264Config->vflip << 1) |
                             (h264Config->hflip));
    /*set venc handle parameters.*/
    vencHandleTemp->encodeStreams[formatPar->streamId].maxWidth  = strFormat->encode_width;
    vencHandleTemp->encodeStreams[formatPar->streamId].maxHeight = strFormat->encode_height;

    return GADI_OK;
}

GADI_ERR gadi_venc_set_stream_format(GADI_SYS_HandleT handle,
                                             GADI_VENC_StreamFormatT* formatPar)
{
    GADI_U32 hFlip, vFlip, rotate;
    GADI_ERR retVal;
    iav_encode_format_ex_t  *strFormat;
    iav_h264_config_ex_t    *h264Config = NULL;
    iav_jpeg_config_ex_t    *mjpegConfig = NULL;
    GADI_VENC_HandleT       *vencHandleTemp = (GADI_VENC_HandleT*)handle;
    GADI_VIN_HandleT        *vinHandleTemp = NULL;
    GADI_VENC_StreamParamsT *encodeStreams = NULL;

    if((vencHandleTemp == NULL) || (vencHandleTemp->fdIav <= 0) || (formatPar == NULL))
    {
        return GADI_VENC_ERR_BAD_PARAMETER;
    }


    vinHandleTemp = (GADI_VIN_HandleT*)vencHandleTemp->vinHandle;

    encodeStreams =  &(vencHandleTemp->encodeStreams[formatPar->streamId]);
    strFormat   = &(vencHandleTemp->encodeStreams[formatPar->streamId].format);
    h264Config  = &(vencHandleTemp->encodeStreams[formatPar->streamId].h264);
    mjpegConfig = &(vencHandleTemp->encodeStreams[formatPar->streamId].mjpeg);

    strFormat->id = 1 << formatPar->streamId;
    strFormat->encode_type   = formatPar->encodeType;
    strFormat->source        = formatPar->sourceBuf;
    strFormat->encode_width  = formatPar->width;
    strFormat->encode_height = formatPar->height;
    strFormat->encode_x      = formatPar->xOffset;
    strFormat->encode_y      = formatPar->yOffset;

    hFlip  = ((formatPar->flipRotate & 0x1) != 0);
    vFlip  = ((formatPar->flipRotate & 0x2) != 0);
    rotate = ((formatPar->flipRotate & 0x4) != 0);

    h264Config->hflip  = hFlip;
    h264Config->vflip  = vFlip;
    h264Config->rotate_clockwise = rotate;
    mjpegConfig->hflip = hFlip;
    mjpegConfig->vflip = vFlip;
    mjpegConfig->rotate_clockwise = rotate;

    if (rotate)
    {
        if (encodeStreams->maxWidth< strFormat->encode_height)
            encodeStreams->maxWidth = strFormat->encode_height;
        if (encodeStreams->maxHeight < strFormat->encode_width)
            encodeStreams->maxHeight = strFormat->encode_width;
    }
    else
    {
        if (encodeStreams->maxWidth < strFormat->encode_width)
            encodeStreams->maxWidth = strFormat->encode_width;
        if (encodeStreams->maxHeight < strFormat->encode_height)
            encodeStreams->maxHeight = strFormat->encode_height;
    }


    /*check resource limit*/
    retVal = venc_check_resource(vencHandleTemp);

    /*set frame rate.*/
    retVal = venc_set_encode_fps(vencHandleTemp,
                                 (1 << formatPar->streamId),
                                 formatPar->fps,
                                 vinHandleTemp->frameRate);
    if(retVal != GADI_OK)
    {
        return retVal;
    }

    if (ioctl(vencHandleTemp->fdIav, IAV_IOC_SET_ENCODE_FORMAT_EX, strFormat) < 0)
    {
        perror("IAV_IOC_SET_ENCODE_FORMAT_EX\n");
        return GADI_VENC_ERR_FROM_DRIVER;
    }

    #if 0
    if (strFormat->encode_type == GADI_VENC_TYPE_H264)
    {
        if (ioctl(vencHandleTemp->fdIav, IAV_IOC_SET_H264_CONFIG_EX, h264Config) < 0)
        {
            perror("IAV_IOC_SET_H264_CONFIG_EX\n");
            return GADI_VENC_ERR_FROM_DRIVER;
        }
    }
    if (strFormat->encode_type == GADI_VENC_TYPE_MJPEG)
    {
        if (ioctl(vencHandleTemp->fdIav, IAV_IOC_SET_JPEG_CONFIG_EX, mjpegConfig) < 0)
        {
            perror("IAV_IOC_SET_JPEG_CONFIG_EX\n");
            return GADI_VENC_ERR_FROM_DRIVER;
        }
    }
    #endif


    return GADI_OK;
}

GADI_ERR gadi_venc_get_h264_config(GADI_SYS_HandleT handle,
                                               GADI_VENC_H264ConfigT* configPar)
{
    GADI_VENC_HandleT    *vencHandleTemp = (GADI_VENC_HandleT*)handle;
    iav_h264_config_ex_t *h264Config = NULL;
    iav_bitrate_info_ex_t bps;

    if((vencHandleTemp == NULL) || (vencHandleTemp->fdIav <= 0) || (configPar == NULL))
    {
        return GADI_VENC_ERR_BAD_PARAMETER;
    }

    h264Config = &(vencHandleTemp->encodeStreams[configPar->streamId].h264);
    h264Config->id = 1 << configPar->streamId;

    if (ioctl(vencHandleTemp->fdIav, IAV_IOC_GET_H264_CONFIG_EX, h264Config) < 0)
    {
        perror("IAV_IOC_GET_H264_CONFIG_EX\n");
        return GADI_VENC_ERR_FROM_DRIVER;
    }

    bps.id = 1 << configPar->streamId;
    if (ioctl(vencHandleTemp->fdIav, IAV_IOC_GET_BITRATE_EX, &bps) < 0)
    {
        perror("IAV_IOC_GET_BITRATE_EX\n");
        return GADI_VENC_ERR_FROM_DRIVER;
    }

    configPar->gopM = h264Config->M;
    configPar->gopN = h264Config->N;
    configPar->idrInterval = h264Config->idr_interval;
    configPar->gopModel = h264Config->gop_model;
    configPar->profile  = h264Config->entropy_codec;
    configPar->brcMode   = bps.rate_control_mode;
    configPar->cbrAvgBps = bps.cbr_avg_bitrate;
    configPar->vbrMinbps = bps.vbr_min_bitrate;
    configPar->vbrMaxbps = bps.vbr_max_bitrate;

    return GADI_OK;
}

GADI_ERR gadi_venc_set_h264_config(GADI_SYS_HandleT handle,
                                               GADI_VENC_H264ConfigT* configPar)
{
    GADI_ERR retVal;
    GADI_VENC_HandleT    *vencHandleTemp = (GADI_VENC_HandleT*)handle;
    iav_h264_config_ex_t *h264Config = NULL;
    iav_bitrate_info_ex_t bps;

    if((vencHandleTemp == NULL) || (vencHandleTemp->fdIav <= 0) || (configPar == NULL))
    {
        return GADI_VENC_ERR_BAD_PARAMETER;
    }

    h264Config = &(vencHandleTemp->encodeStreams[configPar->streamId].h264);

    h264Config->id = 1 << configPar->streamId;
    h264Config->M = configPar->gopM;
    h264Config->N = configPar->gopN;
    h264Config->idr_interval = configPar->idrInterval;
    h264Config->gop_model = configPar->gopModel;
    h264Config->entropy_codec = configPar->profile;
    h264Config->bitrate_control = IAV_BRC_SCBR;
    if (configPar->brcMode & 0x1)
    {
        h264Config->average_bitrate = configPar->vbrMaxbps;
    }
    else
    {
        h264Config->average_bitrate = configPar->cbrAvgBps;
    }

    /*check resource limit*/
    retVal = venc_check_resource(vencHandleTemp);

    if (ioctl(vencHandleTemp->fdIav, IAV_IOC_SET_H264_CONFIG_EX, h264Config) < 0)
    {
        perror("IAV_IOC_SET_H264_CONFIG_EX\n");
        return GADI_VENC_ERR_FROM_DRIVER;
    }

    if (configPar->brcMode >= GADI_VENC_BRC_MODE_NUM)
    {
        GADI_INFO("Invalid bitrate control mode %d.\n", configPar->brcMode);
    }

    bps.id = 1 << configPar->streamId;
    bps.rate_control_mode = configPar->brcMode;
    bps.cbr_avg_bitrate   = configPar->cbrAvgBps;
    bps.vbr_min_bitrate   = configPar->vbrMinbps;
    bps.vbr_max_bitrate   = configPar->vbrMaxbps;

    if (ioctl(vencHandleTemp->fdIav, IAV_IOC_SET_BITRATE_EX, &bps) < 0)
    {
        perror("IAV_IOC_SET_BITRATE_EX\n");
        return GADI_VENC_ERR_FROM_DRIVER;
    }

    return retVal;
}

GADI_ERR gadi_venc_get_mjpeg_config(GADI_SYS_HandleT handle,
                                              GADI_VENC_MjpegConfigT* configPar)
{
    GADI_VENC_HandleT    *vencHandleTemp = (GADI_VENC_HandleT*)handle;
    iav_jpeg_config_ex_t *mjpegConfig = NULL;

    if((vencHandleTemp == NULL) || (vencHandleTemp->fdIav <= 0) || (configPar == NULL))
    {
        return GADI_VENC_ERR_BAD_PARAMETER;
    }

    mjpegConfig = &(vencHandleTemp->encodeStreams[configPar->streamId].mjpeg);

    mjpegConfig->id = 1 << configPar->streamId;
    if(ioctl(vencHandleTemp->fdIav, IAV_IOC_GET_JPEG_CONFIG_EX, mjpegConfig) < 0)
    {
        perror("IAV_IOC_GET_JPEG_CONFIG_EX\n");
        return GADI_VENC_ERR_FROM_DRIVER;
    }

    configPar->chromaFormat = mjpegConfig->chroma_format;
    configPar->quality      = mjpegConfig->quality;

    return GADI_OK;
}

GADI_ERR gadi_venc_set_mjpeg_config(GADI_SYS_HandleT handle,
                                              GADI_VENC_MjpegConfigT* configPar)
{
    GADI_VENC_HandleT    *vencHandleTemp = (GADI_VENC_HandleT*)handle;
    iav_jpeg_config_ex_t *mjpegConfig = NULL;

    if((vencHandleTemp == NULL) || (vencHandleTemp->fdIav <= 0) || (configPar == NULL))
    {
        return GADI_VENC_ERR_BAD_PARAMETER;
    }

    /*check input parameters.*/
    if((configPar->chromaFormat > 1) ||
       (configPar->quality < 1) ||
       (configPar->quality > 100))
    {
        GADI_ERROR("invalid parameters,chromaFormat:0,1,quality:1~100, \
        input chromaFormat:%d,quality:%d",configPar->chromaFormat,configPar->quality);
        return GADI_VENC_ERR_BAD_PARAMETER;
    }

    mjpegConfig = &(vencHandleTemp->encodeStreams[configPar->streamId].mjpeg);

    mjpegConfig->chroma_format = configPar->chromaFormat;
    mjpegConfig->quality       = configPar->quality;

    if (ioctl(vencHandleTemp->fdIav, IAV_IOC_SET_JPEG_CONFIG_EX, mjpegConfig) < 0)
    {
        perror("IAV_IOC_SET_JPEG_CONFIG_EX\n");
        return GADI_VENC_ERR_FROM_DRIVER;
    }

    return GADI_OK;
}

GADI_ERR gadi_venc_get_dptz_param(GADI_SYS_HandleT handle, GADI_VENC_DptzParamT* dptzPar)
{
    GADI_U32 zoomFactor;
    iav_digital_zoom_ex_t dptz;
    iav_digital_zoom2_ex_t dptz2;
    iav_source_buffer_format_ex_t *mainBufFormat = NULL;
    GADI_VENC_HandleT *vencHandleTemp = (GADI_VENC_HandleT*)handle;
    GADI_VIN_HandleT  *vinHandleTemp  = NULL;

    if((vencHandleTemp == NULL) || (vencHandleTemp->fdIav <= 0) || (dptzPar == NULL))
    {
        return GADI_VENC_ERR_BAD_PARAMETER;
    }

    vinHandleTemp = (GADI_VIN_HandleT*)vencHandleTemp->vinHandle;
    mainBufFormat = &(vencHandleTemp->sourceBuffers[GADI_VENC_BUFFER_MAIN].format);

    if (dptzPar->bufferId == GADI_VENC_BUFFER_MAIN)
    {
        if (ioctl(vencHandleTemp->fdIav, IAV_IOC_GET_DIGITAL_ZOOM_EX, &dptz) < 0)
        {
            perror("IAV_IOC_GET_DIGITAL_ZOOM_EX\n");
            return GADI_VENC_ERR_FROM_DRIVER;
        }

        zoomFactor = (vinHandleTemp->vinHeight * dptz.zoom_factor_x) /
                     mainBufFormat->height * 100 / 65536 + 1;

        venc_get_dptz_factor(&(dptzPar->zoomFactor),
                             zoomFactor,
                             dptzPar->bufferId,
                             vencHandleTemp);
        dptzPar->offsetX = dptz.center_offset_x / 65536;
        dptzPar->offsetY = dptz.center_offset_y / 65536;
    }
    else
    {
        dptz2.source = dptzPar->bufferId;
        if (ioctl(vencHandleTemp->fdIav, IAV_IOC_GET_2ND_DIGITAL_ZOOM_EX, &dptz2) < 0)
        {
            perror("IAV_IOC_GET_2ND_DIGITAL_ZOOM_EX\n");
            return GADI_VENC_ERR_FROM_DRIVER;
        }
        zoomFactor = mainBufFormat->height * 100 / dptz2.input_height;
        venc_get_dptz_factor(&(dptzPar->zoomFactor),
                             zoomFactor,
                             dptzPar->bufferId,
                             vencHandleTemp);
        dptzPar->offsetX = dptz2.input_offset_x;
        dptzPar->offsetY = dptz2.input_offset_y;
    }

    return GADI_OK;
}

GADI_ERR gadi_venc_set_dptz_param(GADI_SYS_HandleT handle,
                                              GADI_VENC_DptzParamT* vencDptzPar)
{
    GADI_ERR retVal;
    GADI_VENC_HandleT* vencHandleTemp = (GADI_VENC_HandleT*)handle;

    if((vencHandleTemp == NULL) || (vencHandleTemp->fdIav <= 0) || (vencDptzPar == NULL))
    {
        return GADI_VENC_ERR_BAD_PARAMETER;
    }

    retVal = venc_set_dptz_control_param(vencHandleTemp, vencDptzPar);

    return retVal;
}

GADI_ERR gadi_venc_get_dptz_org_param(GADI_SYS_HandleT handle,
                                                GADI_VENC_DptzParamT *vencDptzPar,
                                                GADI_VENC_DptzOrgParamT *vencDptzOrgPar)
{
    iav_digital_zoom_ex_t dptz;
    GADI_VENC_HandleT *vencHandleTemp = (GADI_VENC_HandleT*)handle;

    if((vencHandleTemp == NULL) || (vencHandleTemp->fdIav <= 0) || (vencDptzOrgPar == NULL))
    {
        return GADI_VENC_ERR_BAD_PARAMETER;
    }

    gadi_sys_memset(&dptz, 0, sizeof(iav_digital_zoom_ex_t));

    if (vencDptzPar == NULL)
    {
        if (vencDptzOrgPar->bufferId == GADI_VENC_BUFFER_MAIN)
        {
            if (ioctl(vencHandleTemp->fdIav, IAV_IOC_GET_DIGITAL_ZOOM_EX, &dptz) < 0)
            {
                perror("IAV_IOC_GET_DIGITAL_ZOOM_EX\n");
                return GADI_VENC_ERR_FROM_DRIVER;
            }
        }
    }
    else
    {
        if (vencDptzOrgPar->bufferId == GADI_VENC_BUFFER_MAIN)
        {
            if (ioctl(vencHandleTemp->fdIav, IAV_IOC_GET_DIGITAL_ZOOM_EX, &dptz) < 0)
            {
                perror("IAV_IOC_GET_DIGITAL_ZOOM_EX\n");
                return GADI_VENC_ERR_FROM_DRIVER;
            }
            if (venc_calc_dptz_I_param(vencDptzPar, &dptz, vencHandleTemp) < 0)
            {
                GADI_ERROR("Calculate DPTZ I parameters failed!\n");
                return GADI_VENC_ERR_FROM_DRIVER;
            }
        }
    }

    vencDptzOrgPar->zoomFactorX = dptz.zoom_factor_x;
    vencDptzOrgPar->zoomFactorY = dptz.zoom_factor_y;
    vencDptzOrgPar->offsetX = dptz.center_offset_x;
    vencDptzOrgPar->offsetY = dptz.center_offset_y;

    return GADI_OK;
}

//*****************************************************************************
//*****************************************************************************
//** Local Functions
//*****************************************************************************
//*****************************************************************************
static GADI_U32 venc_stream_count(GADI_U32 idFormat)
{
    GADI_U32 count = 0;
    if (idFormat & (~STREAM_MASK))
    {
        return 0;
    }
    while (idFormat)
    {
        idFormat = idFormat & (idFormat - 1);
        ++count;
    }
    return count;
}

static GADI_ERR venc_set_encode_fps(GADI_VENC_HandleT* handlePtr,
                                           GADI_U32 idFormat,
                                           GADI_U32 fps,
                                           GADI_U32 vinFrameRate)
{
    GADI_U32  ratioNumerator;
    iav_change_framerate_factor_ex_t change_fr;

    if((handlePtr == NULL) || (handlePtr->fdIav <= 0))
    {
        return GADI_VENC_ERR_BAD_PARAMETER;
    }

    ratioNumerator = (vinFrameRate / 100 * 60) / (fps / 100);
    if (ratioNumerator > 60)
    {
        ratioNumerator = 60;
    }
    change_fr.id = idFormat;
    change_fr.ratio_numerator = ratioNumerator;
    change_fr.ratio_denominator = 60;
    if (ioctl(handlePtr->fdIav, IAV_IOC_CHANGE_FRAMERATE_FACTOR_EX, &change_fr) < 0)
    {
        perror("IAV_IOC_CHANGE_FRAMERATE_FACTOR_EX\n");
        GADI_ERROR("stream format %d, change frame interval %d/%d error\n",
            change_fr.id, change_fr.ratio_numerator, change_fr.ratio_denominator);
        return GADI_VENC_ERR_FROM_DRIVER;
    }

    GADI_INFO("change frame rate factor : %d/60.\n", ratioNumerator);

    return GADI_OK;
}

static GADI_ERR venc_get_dptz_factor(GADI_U32 *newFactor,
                                                GADI_U32 oldFactor,
                                                GADI_U32 bufId,
                                                GADI_VENC_HandleT *vencHandle)
{
    GADI_U32 maxFactor;

    if (bufId== GADI_VENC_BUFFER_MAIN)
    {
        maxFactor = MAX_ZOOM_IN_MAIN_BUFFER;
    }
    else if (bufId == GADI_VENC_BUFFER_SECOND)
    {
        maxFactor = (vencHandle->sourceBuffers[GADI_VENC_BUFFER_MAIN].format.height * 100) /
                    (vencHandle->sourceBuffers[bufId].format.height);
    }
    else
    {
        maxFactor = MAX_ZOOM_IN_PREVIEW_A_B;
    }

    *newFactor = (oldFactor - 100) * MAX_ZOOM_STEPS / (maxFactor - 100) + 1;

    return GADI_OK;
}

static GADI_ERR venc_calc_dptz_I_param(GADI_VENC_DptzParamT *vencDptz,
                                               iav_digital_zoom_ex_t *dptz,
                                               GADI_VENC_HandleT *vencHandle)
{
    const GADI_S32 MAX_ZOOM_FACTOR_Y = (4 << 16);
    const GADI_S32 CENTER_OFFSET_Y_MARGIN = 4;

    GADI_S32 vinW, vinH;
    GADI_S32 maxWindowFactor, maxZoomFactor, factorX, factorY;
    GADI_S32 x, y, maxX, maxY;
    GADI_S32 zoomOutWidth, zoomOutHeight;
    GADI_VIN_HandleT* vinHandleTemp = NULL;
    iav_source_buffer_format_ex_t *mainBufFormat = NULL;

    vinHandleTemp = (GADI_VIN_HandleT*)vencHandle->vinHandle;

    vinW = vinHandleTemp->vinWidth;
    vinH = vinHandleTemp->vinHeight;

    mainBufFormat = &(vencHandle->sourceBuffers[GADI_VENC_BUFFER_MAIN].format);

    if (vencDptz->zoomFactor == 0)
    {
        /*restore to 1X center view*/
        factorY = (mainBufFormat->height << 16) / ROUND_UP(vinH-3,4);
        factorX = factorY;
        x = y = 0;
    }
    else
    {
        maxWindowFactor = vinH * 100 / mainBufFormat->height;
        maxZoomFactor = MAX_ZOOM_IN_MAIN_BUFFER;
        factorY = (vencDptz->zoomFactor - 1) * (maxZoomFactor - 100) / MAX_ZOOM_STEPS + 100;
        factorY = maxWindowFactor * 100 / factorY;
        zoomOutWidth  = ROUND_UP(factorY * mainBufFormat->width / 100, 4);
        zoomOutHeight = ROUND_UP(factorY * mainBufFormat->height / 100, 4);
        factorX = (mainBufFormat->width << 16) / zoomOutWidth;
        factorY = (mainBufFormat->width << 16) / zoomOutHeight;
        x = ((int)dptz->center_offset_x / 65536) + vencDptz->offsetX * zoomOutWidth / 1000;
        y = ((int)dptz->center_offset_y / 65536) + vencDptz->offsetY * zoomOutHeight / 1000;
        maxX = vinW / 2 - zoomOutWidth / 2;
        maxY = vinH / 2 - zoomOutHeight / 2;
        if (factorY >= MAX_ZOOM_FACTOR_Y)
            maxY -= CENTER_OFFSET_Y_MARGIN;
        if (x < -maxX)
            x = -maxX;
        if (x > maxX)
            x = maxX;
        x <<= 16;
        if (y < -maxY)
            y = -maxY;
        if (y > maxY)
            y = maxY;
        y <<= 16;
    }

    dptz->zoom_factor_x = factorX;
    dptz->zoom_factor_y = factorY;
    dptz->center_offset_x = x;
    dptz->center_offset_y = y;

    GADI_INFO("DPTZ Type I : zoom_factor_x 0x%x, zoom_factor_y 0x%x, offset_x 0x%x, offset_y 0x%x.\n",
        dptz->zoom_factor_x, dptz->zoom_factor_y, dptz->center_offset_x, dptz->center_offset_y);

    return GADI_OK;
}

static GADI_ERR venc_calc_dptz_II_param(GADI_VENC_DptzParamT *vencDptz,
                                          iav_digital_zoom2_ex_t *dptz,
                                          GADI_VENC_HandleT *vencHandle)
{
    GADI_S32 x, y;
    GADI_S32 maxZoomFactor, maxWindowFactor, zoomFactor;
    GADI_S32 zoomOutWidth, zoomOutHeight;
    GADI_S32 maxHeightFactor, maxWidthFactor;
    iav_source_buffer_format_ex_t *mainBufFormat = NULL;
    iav_source_buffer_format_ex_t *targetBufFormat = NULL;

    mainBufFormat   = &(vencHandle->sourceBuffers[GADI_VENC_BUFFER_MAIN].format);
    targetBufFormat = &(vencHandle->sourceBuffers[vencDptz->bufferId].format);

    maxHeightFactor = (mainBufFormat->height * 100) / targetBufFormat->height;
    maxWidthFactor  = (mainBufFormat->width * 100) / targetBufFormat->width;
    if (vencDptz->zoomFactor == 0)
    {
        // restore to 1X center view
        if (maxHeightFactor <= maxWidthFactor)
        {
            zoomFactor = maxHeightFactor;
            zoomOutWidth  = zoomFactor * targetBufFormat->width / 100;
            zoomOutHeight = mainBufFormat->height;
            x = (mainBufFormat->width - zoomOutWidth) / 2;
            y = 0;
        }
        else
        {
            zoomFactor = maxWidthFactor;
            zoomOutWidth  = mainBufFormat->width;
            zoomOutHeight = zoomFactor * targetBufFormat->height / 100;
            x = 0;
            y = (mainBufFormat->height - zoomOutHeight) / 2;
        }
    }
    else
    {
        maxWindowFactor = maxHeightFactor > maxWidthFactor ? maxWidthFactor:maxHeightFactor;
        if (vencDptz->bufferId == GADI_VENC_BUFFER_SECOND)
        {
            maxZoomFactor = maxWindowFactor;
        }
        else
        {
            maxZoomFactor = MAX_ZOOM_IN_PREVIEW_A_B;
        }
        zoomFactor = (vencDptz->zoomFactor - 1) * (maxZoomFactor - 100) / MAX_ZOOM_STEPS + 100;
        zoomFactor = maxWindowFactor * 100 / zoomFactor;
        zoomOutWidth  = zoomFactor * targetBufFormat->width / 100;
        zoomOutHeight = zoomFactor * targetBufFormat->height / 100;
        x = dptz->input_offset_x + vencDptz->offsetX* zoomOutWidth / 1000;
        y = dptz->input_offset_y + vencDptz->offsetY * zoomOutHeight / 1000;
        if (x + zoomOutWidth > mainBufFormat->width)
            x = mainBufFormat->width - zoomOutWidth;
        x = (x > 0) ? x : 0;
        if (y + zoomOutHeight > mainBufFormat->height)
            y = mainBufFormat->height - zoomOutHeight;
        y = (y > 0) ? y : 0;
    }

    dptz->source = vencDptz->bufferId;
    dptz->input_width = zoomOutWidth & (~0x1);
    dptz->input_height = zoomOutHeight & (~0x3);
    dptz->input_offset_x = x & (~0x1);
    dptz->input_offset_y = y & (~0x3);

    GADI_INFO("DPTZ Type II : source %d, input window %dx%d, offset %dx%d.\n",
        dptz->source, dptz->input_width, dptz->input_height,
        dptz->input_offset_x, dptz->input_offset_y);

    return GADI_OK;
}

static GADI_ERR venc_set_dptz_control_param(GADI_VENC_HandleT *handle,
                                                  GADI_VENC_DptzParamT *vencDptz)
{
    iav_digital_zoom_ex_t  dptz;
    iav_digital_zoom2_ex_t dptz2;

    if (vencDptz->bufferId == GADI_VENC_BUFFER_MAIN)
    {
        /*get previous dptz parameter*/
        if (ioctl(handle->fdIav, IAV_IOC_GET_DIGITAL_ZOOM_EX, &dptz) < 0)
        {
            perror("IAV_IOC_GET_DIGITAL_ZOOM_EX\n");
            return GADI_VENC_ERR_FROM_DRIVER;
        }
        if (venc_calc_dptz_I_param(vencDptz, &dptz, handle) < 0)
        {
            GADI_ERROR("Calculate DPTZ I parameters failed!\n");
            return GADI_VENC_ERR_FROM_DRIVER;
        }
        if (ioctl(handle->fdIav, IAV_IOC_SET_DIGITAL_ZOOM_EX, &dptz) < 0)
        {
            perror("IAV_IOC_SET_DIGITAL_ZOOM_EX\n");
            return GADI_VENC_ERR_FROM_DRIVER;
        }
    }
    else
    {
        /*get previous dptz parameter*/
        dptz2.source = vencDptz->bufferId;
        if (ioctl(handle->fdIav, IAV_IOC_GET_2ND_DIGITAL_ZOOM_EX, &dptz2) < 0)
        {
            perror("IAV_IOC_GET_2ND_DIGITAL_ZOOM_EX\n");
            return GADI_VENC_ERR_FROM_DRIVER;
        }
        if (venc_calc_dptz_II_param(vencDptz, &dptz2, handle) < 0)
        {
            GADI_ERROR("Calculate DPTZ II parameters failed!\n");
            return GADI_VENC_ERR_FROM_DRIVER;
        }
        if (ioctl(handle->fdIav, IAV_IOC_SET_2ND_DIGITAL_ZOOM_EX, &dptz2) < 0)
        {
            perror("IAV_IOC_SET_2ND_DIGITAL_ZOOM_EX\n");
            return GADI_VENC_ERR_FROM_DRIVER;
        }
    }

    return GADI_OK;
}

static GADI_ERR venc_zoom_to_max_fov_keep_as(GADI_VENC_HandleT *handle,
                                                              GADI_U32 idFormat)
{
    GADI_U32 i;
    GADI_VENC_DptzParamT dptz;

    gadi_sys_memset(&dptz, 0, sizeof(GADI_VENC_DptzParamT));

    for (i = 0; i < GADI_VENC_BUFFER_NUM; ++i)
    {
        if (idFormat & (1 << i))
        {
            dptz.bufferId = handle->encodeStreams[i].format.source;
            venc_set_dptz_control_param(handle, &dptz);
        }
    }

    return GADI_OK;
}

static GADI_ERR venc_check_resource(GADI_VENC_HandleT *venHandle)
{
    iav_system_resource_setup_ex_t sysResource;
    iav_source_buffer_format_ex_t *bufFormatPar = NULL;
    iav_encode_format_ex_t        *strFormatPar = NULL;
    iav_h264_config_ex_t          *h264ConPar;
    GADI_VIN_HandleT              *vinHandleTemp = NULL;
    GADI_U8 needUpdate = 0;
    GADI_U8 strNum = 0;
    GADI_U8 cnt = 0;

    vinHandleTemp  = (GADI_VIN_HandleT*)venHandle->vinHandle;

    if (ioctl(venHandle->fdIav, IAV_IOC_GET_SYSTEM_RESOURCE_LIMIT_EX, &sysResource) < 0)
    {
        perror("IAV_IOC_GET_SYSTEM_RESOURCE_LIMIT_EX\n");
        return GADI_VENC_ERR_FROM_DRIVER;
    }

    /*check source buffer format.*/
    /*check main buffer format.*/
    bufFormatPar = &(venHandle->sourceBuffers[GADI_VENC_BUFFER_MAIN].format);

    if((bufFormatPar->width > vinHandleTemp->vinWidth) ||
       (bufFormatPar->height > vinHandleTemp->vinHeight))
    {
        GADI_ERROR("venc_check_resource:main source buffer %dx%d is larger \
            than video inpu %dx%d", bufFormatPar->width, bufFormatPar->height,
            vinHandleTemp->vinWidth, vinHandleTemp->vinHeight);
        return GADI_VENC_ERR_BAD_PARAMETER;
    }

    if((sysResource.main_source_buffer_max_width  != bufFormatPar->width) ||
       (sysResource.main_source_buffer_max_height != bufFormatPar->height))
    {
        /*reset system source limit.*/
        needUpdate = 1;
        sysResource.main_source_buffer_max_width  = bufFormatPar->width;
        sysResource.main_source_buffer_max_height = bufFormatPar->height;
    }

    /*check second buffer format.*/
    bufFormatPar = &(venHandle->sourceBuffers[GADI_VENC_BUFFER_SECOND].format);
    if((sysResource.second_source_buffer_max_width  != bufFormatPar->width) ||
       (sysResource.second_source_buffer_max_height != bufFormatPar->height))
    {
        /*reset system source limit.*/
        needUpdate = 1;
        sysResource.second_source_buffer_max_width  = bufFormatPar->width;
        sysResource.second_source_buffer_max_height = bufFormatPar->height;
    }

    /*check third buffer format.*/
    bufFormatPar = &(venHandle->sourceBuffers[GADI_VENC_BUFFER_THIRD].format);
    if((sysResource.third_source_buffer_max_width  != bufFormatPar->width) ||
       (sysResource.third_source_buffer_max_height != bufFormatPar->height))
    {
        /*reset system source limit.*/
        needUpdate = 1;
        sysResource.third_source_buffer_max_width  = bufFormatPar->width;
        sysResource.third_source_buffer_max_height = bufFormatPar->height;
    }

    /*check fourth buffer format.*/
    bufFormatPar = &(venHandle->sourceBuffers[GADI_VENC_BUFFER_FOURTH].format);
    if((sysResource.fourth_source_buffer_max_width  != bufFormatPar->width) ||
       (sysResource.fourth_source_buffer_max_height != bufFormatPar->height))
    {
        /*reset system source limit.*/
        needUpdate = 1;
        sysResource.fourth_source_buffer_max_width  = bufFormatPar->width;
        sysResource.fourth_source_buffer_max_height = bufFormatPar->height;
    }

    strNum = 0;
    for(cnt = 0; cnt < GADI_VENC_STREAM_NUM; cnt++)
    {
        /*check stream format.*/
        strFormatPar = &(venHandle->encodeStreams[cnt].format);
        if(sysResource.stream_max_encode_size[cnt].width != strFormatPar->encode_width)
        {
            /*reset system source limit.*/
            needUpdate = 1;
            sysResource.stream_max_encode_size[cnt].width = strFormatPar->encode_width;
        }
        if(sysResource.stream_max_encode_size[cnt].height != strFormatPar->encode_height)
        {
            /*reset system source limit.*/
            needUpdate = 1;
            sysResource.stream_max_encode_size[cnt].height = strFormatPar->encode_height;
        }

        /*check h264 stream GOP.*/
        h264ConPar = &(venHandle->encodeStreams[cnt].h264);
        if(sysResource.stream_max_GOP_N[cnt] != h264ConPar->N)
        {
            /*reset system source limit.*/
            needUpdate = 1;
            sysResource.stream_max_GOP_N[cnt] = h264ConPar->N;
        }
        if(sysResource.stream_max_GOP_M[cnt] != h264ConPar->M)
        {
            /*reset system source limit.*/
            needUpdate = 1;
            sysResource.stream_max_GOP_M[cnt] = h264ConPar->M;
        }

        /*get total encode stream number.*/
        if((strFormatPar->encode_type == GADI_VENC_TYPE_H264) ||
           (strFormatPar->encode_type == GADI_VENC_TYPE_MJPEG))
        {
            strNum++;
        }

    }

    /*check encode stream number*/
    if(sysResource.max_num_encode_streams != strNum)
    {
        /*reset system source limit.*/
        needUpdate = 1;
        sysResource.max_num_encode_streams = strNum;
    }

    if (needUpdate)
    {
        if (ioctl(venHandle->fdIav, IAV_IOC_SET_SYSTEM_RESOURCE_LIMIT_EX, &sysResource) < 0)
        {
            perror("IAV_IOC_SET_SYSTEM_RESOURCE_LIMIT_EX\n");
            return GADI_VENC_ERR_FROM_DRIVER;
        }
    }

    return GADI_OK;
}

static GADI_ERR venc_set_preview_buf(GADI_VENC_HandleT *venHandle)
{
    iav_preview_buffer_format_all_ex_t previewBufFormatAll;
    iav_source_buffer_type_ex_t *bufTypePar = NULL;
    GADI_VOUT_HandleT *voutHandleTemp = NULL;

    voutHandleTemp = (GADI_VOUT_HandleT*)venHandle->voutHandle;

    /*main preview buffer corresponds to the third source buffer, and second
      preview buffer corresponds to the fourth source buffer.*/
    bufTypePar = &(venHandle->sourceBuffers[GADI_VENC_BUFFER_THIRD].type);
    if(*bufTypePar ==  IAV_SOURCE_BUFFER_TYPE_PREVIEW)
    {
        if (ioctl(venHandle->fdIav, IAV_IOC_GET_PREVIEW_BUFFER_FORMAT_ALL_EX, &previewBufFormatAll) < 0)
        {
            perror("IAV_IOC_GET_PREVIEW_BUFFER_FORMAT_ALL_EX\n");
            return GADI_VENC_ERR_FROM_DRIVER;
        }
        previewBufFormatAll.main_preview_width  = voutHandleTemp->videoSize.voutWidth;
        previewBufFormatAll.main_preview_height = voutHandleTemp->videoSize.voutHeight;
        if (ioctl(venHandle->fdIav, IAV_IOC_SET_PREVIEW_BUFFER_FORMAT_ALL_EX, &previewBufFormatAll) < 0)
        {
            perror("IAV_IOC_SET_SOURCE_BUFFER_FORMAT_ALL_EX\n");
            return GADI_VENC_ERR_FROM_DRIVER;
        }
    }

    bufTypePar = &(venHandle->sourceBuffers[GADI_VENC_BUFFER_FOURTH].type);
    if(*bufTypePar ==  IAV_SOURCE_BUFFER_TYPE_PREVIEW)
    {
        if (ioctl(venHandle->fdIav, IAV_IOC_GET_PREVIEW_BUFFER_FORMAT_ALL_EX, &previewBufFormatAll) < 0)
        {
            perror("IAV_IOC_GET_PREVIEW_BUFFER_FORMAT_ALL_EX\n");
            return GADI_VENC_ERR_FROM_DRIVER;
        }
        previewBufFormatAll.second_preview_width = voutHandleTemp->videoSize.voutWidth;
        previewBufFormatAll.second_preview_width = voutHandleTemp->videoSize.voutHeight;
        if (ioctl(venHandle->fdIav, IAV_IOC_SET_PREVIEW_BUFFER_FORMAT_ALL_EX, &previewBufFormatAll) < 0)
        {
            perror("IAV_IOC_SET_SOURCE_BUFFER_FORMAT_ALL_EX\n");
            return GADI_VENC_ERR_FROM_DRIVER;
        }
    }

    return GADI_OK;
}


