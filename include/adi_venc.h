/*!
*****************************************************************************
** \file        adi/include/adi_venc.h
**
** \version     $Id: adi_venc.h 1616 2014-09-19 09:04:38Z fengxuequan $
**
** \brief       ADI video encode module header file
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _ADI_VENC_H_
#define _ADI_VENC_H_

#include "stdio.h"
#include "adi_types.h"

#include <basetypes.h>
#include "ambas_common.h"
#include "iav_drv.h"
#include "iav_drv_ex.h"
#include "ambas_vout.h"


//*****************************************************************************
//*****************************************************************************
//** Defines and Macros
//*****************************************************************************
//*****************************************************************************
/*
*******************************************************************************
** Defines for general error codes of the module.
*******************************************************************************
*/
/*! Bad parameter passed. */
#define GADI_VENC_ERR_BAD_PARAMETER                                          \
                              (GADI_VENC_MODULE_BASE + GADI_ERR_BAD_PARAMETER)
/*! Memory allocation failed. */
#define GADI_VENC_ERR_OUT_OF_MEMORY                                          \
                              (GADI_VENC_MODULE_BASE + GADI_ERR_OUT_OF_MEMORY)
/*! Device already initialised. */
#define GADI_VENC_ERR_ALREADY_INITIALIZED                                    \
                              (GADI_VENC_MODULE_BASE + GADI_ERR_ALREADY_INITIALIZED)
/*! Device not initialised. */
#define GADI_VENC_ERR_NOT_INITIALIZED                                        \
                              (GADI_VENC_MODULE_BASE + GADI_ERR_NOT_INITIALIZED)
/*! Feature or function is not available. */
#define GADI_VENC_ERR_FEATURE_NOT_SUPPORTED                                  \
                              (GADI_VENC_MODULE_BASE + GADI_ERR_FEATURE_NOT_SUPPORTED)
/*! Timeout occured. */
#define GADI_VENC_ERR_TIMEOUT                                                \
                              (GADI_VENC_MODULE_BASE + GADI_ERR_TIMEOUT)
/*! The device is busy, try again later. */
#define GADI_VENC_ERR_DEVICE_BUSY                                            \
                              (GADI_VENC_MODULE_BASE + GADI_ERR_DEVICE_BUSY)
/*! Invalid handle was passed. */
#define GADI_VENC_ERR_INVALID_HANDLE                                         \
                              (GADI_VENC_MODULE_BASE + GADI_ERR_INVALID_HANDLE)
/*! Semaphore could not be created. */
#define GADI_VENC_ERR_SEMAPHORE_CREATE                                       \
                               (GADI_VENC_MODULE_BASE + GADI_ERR_SEMAPHORE_CREATE)
/*! The driver's used version is not supported. */
#define GADI_VENC_ERR_UNSUPPORTED_VERSION                                    \
                               (GADI_VENC_MODULE_BASE + GADI_ERR_UNSUPPORTED_VERSION)
/*! The driver's used version is not supported. */
#define GADI_VENC_ERR_FROM_DRIVER                                            \
                               (GADI_VENC_MODULE_BASE + GADI_ERR_FROM_DRIVER)
/*! The device/handle is not open.. */
#define GADI_VENC_ERR_NOT_OPEN                                               \
                               (GADI_VENC_MODULE_BASE + GADI_ERR_NOT_OPEN)



//*****************************************************************************
//*****************************************************************************
//** Enumerated types
//*****************************************************************************
//*****************************************************************************
/*!
*******************************************************************************
** \brief video encode output stream enum.
*******************************************************************************
*/
typedef enum {
    GADI_VENC_STREAM_MAIN = 0,
    GADI_VENC_STREAM_SECOND,
    GADI_VENC_STREAM_THIRD,
    GADI_VENC_STREAM_FORTH,
    GADI_VENC_STREAM_NUM,
} GADI_VENC_StreamEnumT;

/*!
*******************************************************************************
** \brief video encode stream state enum.
*******************************************************************************
*/
typedef enum {
    /*uninitialized or unconfigured*/
    GADI_VENC_STREAM_STATE_UNKNOWN = 0,
    /*configured ready, but not started encoding yet*/
    GADI_VENC_STREAM_STATE_READY_FOR_ENCODING = 1,
    /*encoding*/
    GADI_VENC_STREAM_STATE_ENCODING = 2,
    /*transition state: starting to encode*/
    GADI_VENC_STREAM_STATE_STARTING = 3,
    /*transition state: stopping encoding*/
    GADI_VENC_STREAM_STATE_STOPPING = 4,
    /*known error*/
    GADI_VENC_STREAM_STATE_ERROR    = 255,
} GADI_VENC_StreamStateEnumT;


/*!
*******************************************************************************
** \brief video encode input source buffer enum.
*******************************************************************************
*/
typedef enum {
    GADI_VENC_BUFFER_MAIN = 0,
    GADI_VENC_BUFFER_SECOND,
    GADI_VENC_BUFFER_THIRD,
    GADI_VENC_BUFFER_FOURTH,
    GADI_VENC_BUFFER_NUM,
} GADI_VENC_BufferEnumT;

/*!
*******************************************************************************
** \brief video encode input source buffer state enum.
*******************************************************************************
*/
typedef enum {
    /*uninitialized or unconfigured*/
    GADI_VENC_SOURCE_BUFFER_STATE_UNKNOWN    = 0,
    /*configured, but not used by any stream to encode*/
    GADI_VENC_SOURCE_BUFFER_STATE_IDLE = 1,
    /*configured and used by at least one stream to encode*/
    GADI_VENC_SOURCE_BUFFER_STATE_BUSY = 2,
    /*known error*/
    GADI_VENC_SOURCE_BUFFER_STATE_ERROR = 255,
}GADI_VENC_BufferStateEnumT;


/*!
*******************************************************************************
** \brief video encode input source buffer type for used enum.
*******************************************************************************
*/
typedef enum {
    /*source buffer disabled*/
	GADI_VENC_SOURCE_BUFFER_TYPE_OFF = 0,
	/*source buffer for encoding*/
	GADI_VENC_SOURCE_BUFFER_TYPE_ENCODE = 1,
	/*source buffer for preview*/
	GADI_VENC_SOURCE_BUFFER_TYPE_PREVIEW = 2,
} GADI_VENC_BufferTypeEnumT;
/*!
*******************************************************************************
** \brief video encode mode enum.
*******************************************************************************
*/
typedef enum {
    GADI_VENC_MODE_NORMAL= 0,
    GADI_VENC_MODE_HIGH_MEGA,
    GADI_VENC_MODE_LOW_DELAY,
    GADI_VENC_MODE_TOTAL_NUM,
} GADI_VENC_ModeEnumT;

/*!
*******************************************************************************
** \brief video encode type enum.
*******************************************************************************
*/
typedef enum {
    GADI_VENC_TYPE_OFF = 0,
    GADI_VENC_TYPE_H264,
    GADI_VENC_TYPE_MJPEG,
} GADI_VENC_TypeEnumT;

/*!
*******************************************************************************
** \brief video encode bit rate control mode enum.
*******************************************************************************
*/
typedef enum {
    GADI_VENC_CBR_MODE = 0,
    GADI_VENC_VBR_MODE,
    GADI_VENC_CBR_QUALITY_MODE,
    GADI_VENC_VBR_QUALITY_MODE,
    GADI_VENC_BRC_MODE_NUM,
}GADI_VENC_BrcModeEnumT;


//*****************************************************************************
//*****************************************************************************
//** Data Structures
//*****************************************************************************
//*****************************************************************************
/*!
*******************************************************************************
** \brief one source buffer state struct.
*******************************************************************************
*/
typedef struct
{
    /*buffer index.*/
    GADI_U32            bufferId;
    /*buffer state.*/
    GADI_U32            state;
} GADI_VENC_BufferStateT;

/*!
*******************************************************************************
** \brief one buffer format struct.
*******************************************************************************
*/
typedef struct
{
    /*buffer index.*/
    GADI_U32            bufferId;
    /*source buffer width.*/
    GADI_U16            width;
    /*source buffer height.*/
    GADI_U16            height;
    /*deintlc mode of source buffer.*/
    GADI_U16            deintlcForIntlcVin;
} GADI_VENC_BufferFormatT;

/*!
*******************************************************************************
** \brief all source buffer format struct.
*******************************************************************************
*/
typedef struct
{
    /*main source buffer width.*/
    GADI_U16            mainWidth;
    /*main source buffer height.*/
    GADI_U16            mainHeight;
    /*deintlc mode of main source buffer.*/
    GADI_U16            mainDeintlcForIntlcVin;
    /*second source buffer width.*/
    GADI_U16            secondWidth;
    /*second source buffer height.*/
    GADI_U16            secondHeight;
    /*deintlc mode of second source buffer.*/
    GADI_U16            secondDeintlcForIntlcVin;
    /*third source buffer width.*/
    GADI_U16            thirdWidth;
    /*third source buffer height.*/
    GADI_U16            thirdHeight;
    /*deintlc mode of third source buffer.*/
    GADI_U16            thirdDeintlcForIntlcVin;
    /*fourth source buffer width.*/
    GADI_U16            fourthWidth;
    /*third source buffer height.*/
    GADI_U16            fourthHeight;
    /*deintlc mode of fourth source buffer.*/
    GADI_U16            fourthDeintlcForIntlcVin;
    /*0: OFF  1: use progressive VIN to encode interlaced video*/
    GADI_U8         intlcScan;
}GADI_VENC_BufferFormatAllT;


/*!
*******************************************************************************
** \brief one buffer type struct.
*******************************************************************************
*/
typedef struct
{
    /*buffer index.*/
    GADI_U32            bufferId;
    /*source buffer type 0:disable, 1:for encoding, 2:for preview.*/
    GADI_U32            bufferType;
} GADI_VENC_BufferTypeT;

/*!
*******************************************************************************
** \brief all source buffer format struct.
*******************************************************************************
*/
typedef struct
{
    /*main source buffer(capture buffer) type 0:disable, 1:for encoding, 2:for preview.*/
    GADI_U32            mainBufType;
    /*second source buffer(preview C buffer) type.*/
    GADI_U32            secondBufType;
    /*third source buffer(preview B buffer) type.*/
    GADI_U32            thirdBufType;
    /*fourth source buffer(preview A buffer) type.*/
    GADI_U32            fourthBufType;
}GADI_VENC_BufferTypeAllT;


/*!
*******************************************************************************
** \brief one stream state struct.
*******************************************************************************
*/
typedef struct
{
    /*stream index.*/
    GADI_U32            streamId;
    /*stream state.*/
    GADI_U32            state;
} GADI_VENC_StreamStateT;

/*!
*******************************************************************************
** \brief one stream format struct.
*******************************************************************************
*/
typedef struct
{
    /*stream index.*/
    GADI_U32            streamId;
    /*0: none, 1: H.264, 2: MJPEG*/
    GADI_U8             encodeType;
    /*0: main source buffer (IAV_ENCODE_SOURCE_MAIN_BUFFER)    1: secondary source buffer*/
    GADI_U8             sourceBuf;
    /*rotate*/
    GADI_U8             flipRotate;
    /*encode width.*/
    GADI_U16            width;
    /*encode height.*/
    GADI_U16            height;
    /*encode x offset.*/
    GADI_U16            xOffset;
    /*encode y offset.*/
    GADI_U16            yOffset;
    /*encode frame rate.*/
    GADI_U32            fps;
}GADI_VENC_StreamFormatT;

/*!
*******************************************************************************
** \brief one h264 stream configuration struct.
*******************************************************************************
*/
typedef struct
{
    /*stream index.*/
    GADI_U32            streamId;
    /*gop M value.*/
    GADI_U8             gopM;
    /*gop N value.*/
    GADI_U8             gopN;
    /*IDR interval .*/
    GADI_U8             idrInterval;
    /*gop model.*/
    GADI_U8             gopModel;
    /*encode profile.*/
    GADI_U8             profile;
    /*0: CBR; 1: VBR; 2: CBR keep quality; 3: VBR keep quality: GADI_VENC_BrcModeEnumT*/
    GADI_U8             brcMode;
    /*cbr mode, bit rate.*/
    GADI_U32            cbrAvgBps;
    /*vbr mode, min bit rate.*/
    GADI_U32            vbrMinbps;
    /*vbr mode, max bit rate.*/
    GADI_U32            vbrMaxbps;
}GADI_VENC_H264ConfigT;

/*!
*******************************************************************************
** \brief one MJPEG stream configuration struct.
*******************************************************************************
*/
typedef struct
{
    /*stream index.*/
    GADI_U32            streamId;
    /*0: YUV 422, 1: YUV 420.*/
    GADI_U8             chromaFormat;
    /*1 ~ 100, 100 is best quality.*/
    GADI_U8             quality;
} GADI_VENC_MjpegConfigT;

/*!
*******************************************************************************
** \brief source buffer parameters struct.
*******************************************************************************
*/
typedef struct
{
    /*source buffer information(id & state).*/
    iav_source_buffer_info_ex_t     info;
    /*source buffer information(in/out put resolution).*/
    iav_source_buffer_format_ex_t   format;
    /*source buffer for encode/preview(video output).*/
    iav_source_buffer_type_ex_t     type;
}GADI_VENC_BufferParamsT;

/*!
*******************************************************************************
** \brief video encode stream parameters struct.
*******************************************************************************
*/
typedef struct
{
    /*encode video max width.*/
    GADI_U32                        maxWidth;
    /*encode video max height.*/
    GADI_U32                        maxHeight;
    /*one encode stream information.*/
    iav_encode_stream_info_ex_t     info;
    /*one encode stream  format.*/
    iav_encode_format_ex_t          format;
    /*h264 stream configuration when encode stream format is h264.*/
    iav_h264_config_ex_t            h264;
    /*h264 stream configuration when encode stream format is mjpeg.*/
    iav_jpeg_config_ex_t            mjpeg;

}GADI_VENC_StreamParamsT;

/*!
*******************************************************************************
** \brief video encode module open parameters struct.
*******************************************************************************
*/
typedef struct
{
    /*video input module handle.*/
    GADI_SYS_HandleT            vinHandle;
    /*video output module handle.*/
    GADI_SYS_HandleT            voutHandle;
    /*default source buffer state parameters.*/
    GADI_VENC_BufferStateT      defBufState[GADI_VENC_BUFFER_NUM];
    /*default source buffer format parameters.*/
    GADI_VENC_BufferFormatT     defBufFormat[GADI_VENC_BUFFER_NUM];
    /*default source buffer type parameters.*/
    GADI_VENC_BufferTypeT       defBufType[GADI_VENC_BUFFER_NUM];
    /*defualt encode streams state parameters.*/
    GADI_VENC_StreamStateT      defStrState[GADI_VENC_STREAM_NUM];
    /*defualt encode streams format parameters.*/
    GADI_VENC_StreamFormatT     defStrFormat[GADI_VENC_STREAM_NUM];
    /*defualt encode streams h264 config parameters.*/
    GADI_VENC_H264ConfigT       defH264Config[GADI_VENC_STREAM_NUM];
    /*defualt encode streams config parameters.*/
    GADI_VENC_MjpegConfigT      defMjpegConfig[GADI_VENC_STREAM_NUM];

}GADI_VENC_OpenParamsT;


/*!
*******************************************************************************
** \brief setting video encode bitrate range parameters struct.
*******************************************************************************
*/
typedef struct
{
    /*stream index for setting the bitrate.*/
    GADI_U32            streamId;
    /*VBR mode: the min bitrate, CBR mode: the average bitrate.*/
    GADI_U32            minBps;
    /*VBR mode: the max bitrate.*/
    GADI_U32            maxBps;
}GADI_VENC_BitRateRangeT;

/*!
*******************************************************************************
** \brief settting video encode frame rate parameters struct.
*******************************************************************************
*/
typedef struct
{
    /*stream index for setting the bitrate.*/
    GADI_U32            streamId;
    /*encoding stream target frame rate value.*/
    GADI_U32            fps;
}GADI_VENC_FrameRateT;


/*!
*******************************************************************************
** \brief digital PTZ setting parameters struct.
*******************************************************************************
*/
typedef struct
{
    /*source buffer index.*/
    GADI_U32            bufferId;
    /*zoom factor.*/
    GADI_U32            zoomFactor;
    /*zoom offset x.*/
    GADI_S32            offsetX;
    /*zoom offset y.*/
    GADI_S32            offsetY;
}GADI_VENC_DptzParamT;

/*!
*******************************************************************************
** \brief getting digital PTZ orginal information parameters struct.
*******************************************************************************
*/
typedef struct
{
    /*source buffer index.*/
    GADI_U32            bufferId;
    /*zoom x factor.*/
    GADI_U32            zoomFactorX;
    /*zoom y factor.*/
    GADI_U32            zoomFactorY;
    /*zoom offset x.*/
    GADI_S32            offsetX;
    /*zoom offset y.*/
    GADI_S32            offsetY;
}GADI_VENC_DptzOrgParamT;


//*****************************************************************************
//*****************************************************************************
//** Global Data
//*****************************************************************************
//*****************************************************************************



//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************

#ifdef __cplusplus
extern "C" {
#endif


/*!
*******************************************************************************
** \brief Initialize the ADI video encode module.
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_OUT_OF_MEMORY
** - #GADI_VENC_ERR_ALREADY_INITIALIZED
**
** \sa gadi_vin_exit
**
*******************************************************************************
*/
GADI_ERR gadi_venc_init(void);

/*!
*******************************************************************************
** \brief Shutdown the ADI video encode module.
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_NOT_INITIALIZED
**
** \sa gadi_vin_init
**
*******************************************************************************
*/
GADI_ERR gadi_venc_exit(void);

/*!
*******************************************************************************
** \brief Open one ADI video venc module instance.
**
** \param[in] openParams a struct pointer of open prameters.
** \param[in] errorCodePtr pointer to return the error code.
**
** \return Return an valid handle of ADI vin module instance.
**
** \sa gadi_venc_close
**
*******************************************************************************
*/
GADI_SYS_HandleT gadi_venc_open(GADI_VENC_OpenParamsT* openParams, GADI_ERR* errorCodePtr);

/*!
*******************************************************************************
** \brief close one ADI video encode module instance.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_open
**
*******************************************************************************
*/
GADI_ERR gadi_venc_close(GADI_SYS_HandleT handle);

/*!
*******************************************************************************
** \brief close one ADI video encode module instance.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_open
**
*******************************************************************************
*/
GADI_ERR gadi_venc_close(GADI_SYS_HandleT handle);

/*!
*******************************************************************************
** \brief get current encode mode.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]  *encodeMode pointer of encode mode, enum:GADI_VENC_ModeEnumT;
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_open
**
*******************************************************************************
*/
GADI_ERR gadi_venc_get_encode_mode(GADI_SYS_HandleT handle, GADI_U32* encodeMode);

/*!
*******************************************************************************
** \brief set encode mode.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]  encodeMode encode mode, enum:GADI_VENC_ModeEnumT;
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_open
**
*******************************************************************************
*/
GADI_ERR gadi_venc_set_encode_mode(GADI_SYS_HandleT handle, GADI_U32 encodeMode);

/*!
*******************************************************************************
** \brief start encoding one stream.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]  streamId stream id enum:GADI_VENC_StreamEnumT;
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_FROM_DRIVER
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_stop_stream
**
*******************************************************************************
*/
GADI_ERR gadi_venc_start_stream(GADI_SYS_HandleT handle, GADI_U32 streamId);

/*!
*******************************************************************************
** \brief stop encoding one stream.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]  streamId stream id enum:GADI_VENC_StreamEnumT;
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_FROM_DRIVER
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_start_stream
**
*******************************************************************************
*/
GADI_ERR gadi_venc_stop_stream(GADI_SYS_HandleT handle, GADI_U32 streamId);

/*!
*******************************************************************************
** \brief force one stream IDR .
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]  streamId stream id enum:GADI_VENC_StreamEnumT;
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_FROM_DRIVER
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_open
**
*******************************************************************************
*/
GADI_ERR gadi_venc_force_idr(GADI_SYS_HandleT handle, GADI_U32 streamId);

/*!
*******************************************************************************
** \brief set one stream bit rate.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]  brPar pointer of setting bitrate struct;
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_FROM_DRIVER
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_open
**
*******************************************************************************
*/
GADI_ERR gadi_venc_set_bitrate(GADI_SYS_HandleT handle, GADI_VENC_BitRateRangeT* brPar);

/*!
*******************************************************************************
** \brief set one stream frame rate.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]  frPar pointer of setting frame rate struct;
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_FROM_DRIVER
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_open
**
*******************************************************************************
*/
GADI_ERR gadi_venc_set_framerate(GADI_SYS_HandleT handle, GADI_VENC_FrameRateT* frPar);

/*!
*******************************************************************************
** \brief set one source buffer state.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[out]  statePar pointer of source buffer state struct;
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_FROM_DRIVER
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_open
**
*******************************************************************************
*/
GADI_ERR gadi_venc_get_buffer_state(GADI_SYS_HandleT handle, GADI_VENC_BufferStateT* statePar);

/*!
*******************************************************************************
** \brief get all(four) source buffer format.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[out]  formatAllPar pointer of all source buffer format struct;
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_FROM_DRIVER
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_open
**
*******************************************************************************
*/
GADI_ERR gadi_venc_get_buffer_format_all(GADI_SYS_HandleT handle,
                                        GADI_VENC_BufferFormatAllT* formatAllPar);

/*!
*******************************************************************************
** \brief set all(four) source buffer format.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]  formatAllPar pointer of all source buffer format struct;
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_FROM_DRIVER
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_open
**
*******************************************************************************
*/
GADI_ERR gadi_venc_set_buffer_format_all(GADI_SYS_HandleT handle,
                                       GADI_VENC_BufferFormatAllT* formatAllPar);

/*!
*******************************************************************************
** \brief get all(four) source buffer type.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[out]  typeAllPar pointer of all source buffer type struct;
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_FROM_DRIVER
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_open
**
*******************************************************************************
*/
GADI_ERR gadi_venc_get_buffer_type_all(GADI_SYS_HandleT handle,
                                            GADI_VENC_BufferTypeAllT* typeAllPar);

/*!
*******************************************************************************
** \brief set all(four) source buffer type.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[out]  typeAllPar pointer of all source buffer type struct;
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_FROM_DRIVER
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_open
**
*******************************************************************************
*/
GADI_ERR gadi_venc_set_buffer_type_all(GADI_SYS_HandleT handle,
                                           GADI_VENC_BufferTypeAllT* typeAllPar);

/*!
*******************************************************************************
** \brief get one encode stream state.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[out]  statePar pointer of stream state struct;
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_FROM_DRIVER
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_open
**
*******************************************************************************
*/
GADI_ERR gadi_venc_get_stream_state(GADI_SYS_HandleT handle,
                                               GADI_VENC_StreamStateT* statePar);

/*!
*******************************************************************************
** \brief get one encode stream format.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[out]  formatPar pointer of encode stream format struct;
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_FROM_DRIVER
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_open
**
*******************************************************************************
*/
GADI_ERR gadi_venc_get_stream_format(GADI_SYS_HandleT handle,
                                             GADI_VENC_StreamFormatT *formatPar);

/*!
*******************************************************************************
** \brief set one encode stream format.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]  formatPar pointer of encode stream format struct;
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_FROM_DRIVER
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_open
**
*******************************************************************************
*/
GADI_ERR gadi_venc_set_stream_format(GADI_SYS_HandleT handle,
                                             GADI_VENC_StreamFormatT* formatPar);

/*!
*******************************************************************************
** \brief get one h264 encode stream configuration.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[out]  configPar pointer of h264 stream configuration struct;
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_FROM_DRIVER
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_open
**
*******************************************************************************
*/
GADI_ERR gadi_venc_get_h264_config(GADI_SYS_HandleT handle,
                                               GADI_VENC_H264ConfigT* configPar);

/*!
*******************************************************************************
** \brief set one h264 encode stream configuration.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]  configPar pointer of h264 stream configuration struct;
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_FROM_DRIVER
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_open
**
*******************************************************************************
*/
GADI_ERR gadi_venc_set_h264_config(GADI_SYS_HandleT handle,
                                               GADI_VENC_H264ConfigT* configPar);

/*!
*******************************************************************************
** \brief get one mjpeg encode stream configuration.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[out]  configPar pointer of mjpeg stream configuration struct;
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_FROM_DRIVER
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_open
**
*******************************************************************************
*/
GADI_ERR gadi_venc_get_mjpeg_config(GADI_SYS_HandleT handle,
                                              GADI_VENC_MjpegConfigT* configPar);

/*!
*******************************************************************************
** \brief set one mjpeg encode stream configuration.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]  configPar pointer of mjpeg stream configuration struct;
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_FROM_DRIVER
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_open
**
*******************************************************************************
*/
GADI_ERR gadi_venc_set_mjpeg_config(GADI_SYS_HandleT handle,
                                              GADI_VENC_MjpegConfigT* configPar);

/*!
*******************************************************************************
** \brief get digital ptz parameter.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[out]  dptzPar pointer of digital ptz struct;
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_FROM_DRIVER
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_open
**
*******************************************************************************
*/
GADI_ERR gadi_venc_get_dptz_param(GADI_SYS_HandleT handle, GADI_VENC_DptzParamT* dptzPar);

/*!
*******************************************************************************
** \brief set digital ptz parameter.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]  dptzPar pointer of digital ptz struct;
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_FROM_DRIVER
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_open
**
*******************************************************************************
*/
GADI_ERR gadi_venc_set_dptz_param(GADI_SYS_HandleT handle,
                                              GADI_VENC_DptzParamT* vencDptzPar);

/*!
*******************************************************************************
** \brief get digital ptz orginal parameter.
**
** \param[in]  handle     Valid ADI venc instance handle previously opened by
**                        #gadi_venc_open.
** \param[in]   vencDptzPar
** \param[out]  vencDptzOrgPar pointer of digital ptz orginal parameters struct;
**
** \return
** - #GADI_OK
** - #GADI_VENC_ERR_FROM_DRIVER
** - #GADI_VENC_ERR_BAD_PARAMETER
**
** \sa gadi_venc_open
**
*******************************************************************************
*/
GADI_ERR gadi_venc_get_dptz_org_param(GADI_SYS_HandleT handle,
                                                GADI_VENC_DptzParamT *vencDptzPar,
                                                GADI_VENC_DptzOrgParamT *vencDptzOrgPar);

#ifdef __cplusplus
}
#endif



#endif /* _ADI_VENC_H_ */

