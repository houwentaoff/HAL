/*!
*****************************************************************************
** \file        ./adi/inc/adi_vin.h
**
** \version     $Id: adi_vin.h 1616 2014-09-19 09:04:38Z fengxuequan $
**
** \brief       adi video input porting.
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _ADI_VIN_H_
#define _ADI_VIN_H_

#include "stdio.h"
#include "adi_types.h"


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
#define GADI_VIN_ERR_BAD_PARAMETER                                          \
                              (GADI_VIN_MODULE_BASE + GADI_ERR_BAD_PARAMETER)
/*! Memory allocation failed. */
#define GADI_VIN_ERR_OUT_OF_MEMORY                                          \
                              (GADI_VIN_MODULE_BASE + GADI_ERR_OUT_OF_MEMORY)
/*! Device already initialised. */
#define GADI_VIN_ERR_ALREADY_INITIALIZED                                    \
                              (GADI_VIN_MODULE_BASE + GADI_ERR_ALREADY_INITIALIZED)
/*! Device not initialised. */
#define GADI_VIN_ERR_NOT_INITIALIZED                                        \
                              (GADI_VIN_MODULE_BASE + GADI_ERR_NOT_INITIALIZED)
/*! Feature or function is not available. */
#define GADI_VIN_ERR_FEATURE_NOT_SUPPORTED                                  \
                              (GADI_VIN_MODULE_BASE + GADI_ERR_FEATURE_NOT_SUPPORTED)
/*! Timeout occured. */
#define GADI_VIN_ERR_TIMEOUT                                                \
                              (GADI_VIN_MODULE_BASE + GADI_ERR_TIMEOUT)
/*! The device is busy, try again later. */
#define GADI_DVIN_ERR_DEVICE_BUSY                                            \
                              (GADI_VIN_MODULE_BASE + GADI_ERR_DEVICE_BUSY)
/*! Invalid handle was passed. */
#define GADI_DVIN_ERR_INVALID_HANDLE                                         \
                              (GADI_VIN_MODULE_BASE + GADI_ERR_INVALID_HANDLE)
/*! Semaphore could not be created. */
#define GADI_VIN_ERR_SEMAPHORE_CREATE                                       \
                               (GADI_VIN_MODULE_BASE + GADI_ERR_SEMAPHORE_CREATE)
/*! The driver's used version is not supported. */
#define GADI_VIN_ERR_UNSUPPORTED_VERSION                                    \
                               (GADI_VIN_MODULE_BASE + GADI_ERR_UNSUPPORTED_VERSION)
/*! The driver's used version is not supported. */
#define GADI_VIN_ERR_FROM_DRIVER                                            \
                               (GADI_VIN_MODULE_BASE + GADI_ERR_FROM_DRIVER)
/*! The device/handle is not open.. */
#define GADI_VIN_ERR_NOT_OPEN                                               \
                               (GADI_VIN_MODULE_BASE + GADI_ERR_NOT_OPEN)


//*****************************************************************************
//*****************************************************************************
//** Enumerated types
//*****************************************************************************
//*****************************************************************************




//*****************************************************************************
//*****************************************************************************
//** Data Structures
//*****************************************************************************
//*****************************************************************************

/*!
*******************************************************************************
** \brief video input mirror mode struct.
*******************************************************************************
*/
typedef struct
{
    /*enum amba_vin_src_mirror_pattern pattern*/
    GADI_S32        mirrorPattern;
    /*enum amba_vin_src_bayer_pattern bayer_pattern*/
    GADI_S32        bayerPattern;
} GADI_VIN_MirrorModeParamsT;

/*!
*******************************************************************************
** \brief video input open parameters struct.
*******************************************************************************
*/
typedef struct
{
    /*video input resolution mode: enum amba_video_mode*/
    GADI_U32                    mode;
    /*input video frame rate.*/
    GADI_U32                    frameRate;
    /*only for sensor that output YUV data, 0:no-anti; 1:50Hz; 2:60Hz*/
    GADI_S32                    antiFlicker;
    /*AMBA_VIN_SRC_DEV_TYPE_CCD/AMBA_VIN_SRC_DEV_TYPE_DECODER*/
    GADI_S32                    sensorType;
    /*input video mirror mode.*/
    GADI_VIN_MirrorModeParamsT  mirrorMode;
} GADI_VIN_OpenParamsT;

/*!
*******************************************************************************
** \brief video input setting parameters struct.
*******************************************************************************
*/
typedef struct
{
    /*input ideo resolution mode: enum amba_video_mode*/
    GADI_U32                    resoluMode;
    /*input video frame rate.*/
    GADI_U32                    frameRate;
    /*input video mirror mode.*/
    GADI_VIN_MirrorModeParamsT  mirrorMode;
} GADI_VIN_SettingParamsT;


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
** \brief Initialize the ADI video input module.
**
** \return
** - #GADI_OK
** - #GADI_DSP_ERR_ALREADY_INITIALIZED
** - #GADI_DSP_ERR_OUT_OF_MEMORY
**
** \sa gadi_vin_exit
**
*******************************************************************************
*/
GADI_ERR gadi_vin_init(void);

/*!
*******************************************************************************
** \brief Shutdown the ADI video input module.
**
** \return
** - #GADI_OK
** - #GADI_DSP_ERR_NOT_INITIALIZED
**
** \sa gadi_vin_init
**
*******************************************************************************
*/
GADI_ERR gadi_vin_exit(void);

/*!
*******************************************************************************
** \brief Open one ADI video input module instance.
**
** \param[in] openParams a struct pointer of open prameters.
** \param[in] errorCodePtr pointer to return the error code.
**
** \return Return an valid handle of ADI vin module instance.
**
** \sa gadi_vin_close
**
*******************************************************************************
*/
GADI_SYS_HandleT gadi_vin_open(GADI_VIN_OpenParamsT* openParams, GADI_ERR* errorCodePtr);

/*!
*******************************************************************************
** \brief close one ADI video input module instance.
**
** \param[in]  handle     Valid ADI vin instance handle previously opened by
**                        #gadi_vin_open.
**
** \return
** - #GADI_OK
** - #GADI_VIN_ERR_BAD_PARAMETER
**
** \sa gadi_vin_open
**
*******************************************************************************
*/
GADI_ERR gadi_vin_close(GADI_SYS_HandleT handle);

/*!
*******************************************************************************
** \brief get video input resolution.
**
** \param[in]  handle     Valid dsp ADI instance handle previously opened by
**                        #gadi_vin_open.
** \param[in]  vinWidth   pointer to return the video input width.
** \param[in]  vinHeight  pointer to return the video input height.
**
** \return
** - #GADI_OK
** - #GADI_VIN_ERR_NOT_OPEN
** - #GADI_VIN_ERR_FROM_DRIVER
**
** \sa gadi_vin_open
**
*******************************************************************************
*/
GADI_ERR gadi_vin_get_resolution(GADI_SYS_HandleT handle, GADI_U32* vinWidth, GADI_U32* vinHeight);

/*!
*******************************************************************************
** \brief get video input information.
**
** \param[in]  handle     Valid dsp ADI instance handle previously opened by
**                        #gadi_vin_open.
** \param[in]  vinSettingParams   pointer to return the video input information struct.
**
** \return
** - #GADI_OK
** - #GADI_VIN_ERR_NOT_OPEN
** - #GADI_VIN_ERR_FROM_DRIVER
**
** \sa gadi_vin_open
**
*******************************************************************************
*/
GADI_ERR gadi_vin_get_params(GADI_SYS_HandleT handle, GADI_VIN_SettingParamsT* vinSettingParams);


/*!
*******************************************************************************
** \brief set video input information.
**
** \param[in]  handle     Valid dsp ADI instance handle previously opened by
**                        #gadi_vin_open.
** \param[in]  vinSettingParams   input pointer of video input information struct.
**
** \return
** - #GADI_OK
** - #GADI_VIN_ERR_NOT_OPEN
** - #GADI_VIN_ERR_FROM_DRIVER
** - #GADI_VIN_ERR_BAD_PARAMETER
**
** \sa gadi_vin_open
**
*******************************************************************************
*/
GADI_ERR gadi_vin_set_params(GADI_SYS_HandleT handle, GADI_VIN_SettingParamsT* vinSettingParams);

/*!
*******************************************************************************
** \brief get video input information.
**
** \param[in]  handle    Valid dsp ADI instance handle previously opened by
**                        #gadi_vin_open.
** \param[in]  enable   flag of enter or exit iav preview state, 1: enter, 0: exit.
**
** \return
** - #GADI_OK
** - #GADI_VIN_ERR_NOT_OPEN
** - #GADI_VIN_ERR_FROM_DRIVER
** - #GADI_VIN_ERR_BAD_PARAMETER
**
** \sa gadi_vin_open
**
*******************************************************************************
*/
GADI_ERR gadi_vin_enter_preview(GADI_SYS_HandleT handle, GADI_U32 enable);

#ifdef __cplusplus
    }
#endif


#endif /* _ADI_VIN_H_ */
