/*!
*****************************************************************************
** \file        ./adi/inc/adi_vout.h
**
** \version     $Id: adi_vout.h 1616 2014-09-19 09:04:38Z fengxuequan $
**
** \brief       adi video output porting.
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _ADI_VOUT_H_
#define _ADI_VOUT_H_

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
#define GADI_VOUT_ERR_BAD_PARAMETER                                          \
                              (GADI_VOUT_MODULE_BASE + GADI_ERR_BAD_PARAMETER)
/*! Memory allocation failed. */
#define GADI_VOUT_ERR_OUT_OF_MEMORY                                          \
                              (GADI_VOUT_MODULE_BASE + GADI_ERR_OUT_OF_MEMORY)
/*! Device already initialised. */
#define GADI_VOUT_ERR_ALREADY_INITIALIZED                                    \
                              (GADI_VOUT_MODULE_BASE + GADI_ERR_ALREADY_INITIALIZED)
/*! Device not initialised. */
#define GADI_VOUT_ERR_NOT_INITIALIZED                                        \
                              (GADI_VOUT_MODULE_BASE + GADI_ERR_NOT_INITIALIZED)
/*! Feature or function is not available. */
#define GADI_VOUT_ERR_FEATURE_NOT_SUPPORTED                                  \
                              (GADI_VOUT_MODULE_BASE + GADI_ERR_FEATURE_NOT_SUPPORTED)
/*! Timeout occured. */
#define GADI_VOUT_ERR_TIMEOUT                                                \
                              (GADI_VOUT_MODULE_BASE + GADI_ERR_TIMEOUT)
/*! The device is busy, try again later. */
#define GADI_VOUT_ERR_DEVICE_BUSY                                            \
                              (GADI_VOUT_MODULE_BASE + GADI_ERR_DEVICE_BUSY)
/*! Invalid handle was passed. */
#define GADI_VOUT_ERR_INVALID_HANDLE                                         \
                              (GADI_VOUT_MODULE_BASE + GADI_ERR_INVALID_HANDLE)
/*! Semaphore could not be created. */
#define GADI_VOUT_ERR_SEMAPHORE_CREATE                                       \
                               (GADI_VOUT_MODULE_BASE + GADI_ERR_SEMAPHORE_CREATE)
/*! The driver's used version is not supported. */
#define GADI_VOUT_ERR_UNSUPPORTED_VERSION                                    \
                               (GADI_VOUT_MODULE_BASE + GADI_ERR_UNSUPPORTED_VERSION)
/*! The driver's used version is not supported. */
#define GADI_VOUT_ERR_FROM_DRIVER                                            \
                               (GADI_VOUT_MODULE_BASE + GADI_ERR_FROM_DRIVER)
/*! The device/handle is not open.. */
#define GADI_VOUT_ERR_NOT_OPEN                                               \
                               (GADI_VOUT_MODULE_BASE + GADI_ERR_NOT_OPEN)


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
** \brief video output resolution struct.
*******************************************************************************
*/
typedef struct
{
    /*string of resolution nams.*/
    const char          *name;
    /*video output resolution mode:enum amba_video_mode*/
    GADI_U32            mode;
    /*resolution width.*/
    GADI_S16            width;
    /*resolution height.*/
    GADI_S16            height;
} GADI_VOUT_ResolutionT;

/*!
*******************************************************************************
** \brief video output size struct.
*******************************************************************************
*/
typedef struct
{
    /*flag to specify the video out size.*/
    GADI_U32 specified;
    /*video out width.*/
    GADI_U16 voutWidth;
    /*video out height.*/
    GADI_U16 voutHeight;
    /*video out picutre width.*/
    GADI_U16 videoWidth;
    /*video out picutre height.*/
    GADI_U16 videoHeight;
}GADI_VOUT_VideoSizeT;

/*!
*******************************************************************************
** \brief video output offset struct.
*******************************************************************************
*/
typedef struct
{
    /*flag to specify the video offset.*/
    GADI_U16 specified;
    /*x offset.*/
    GADI_S16 offsetX;
    /*y offset.*/
    GADI_S16 offsetY;
}GADI_VOUT_VideoOffsetT;

/*!
*******************************************************************************
** \brief video output open parameters struct.
*******************************************************************************
*/
typedef struct
{
    /*video output resolution mode:enum amba_video_mode*/
    GADI_U32                    resoluMode;
    /*video output frame rate.*/
    GADI_U32                    frameRate;
    /*specify external device type for vout.*/
    GADI_U32                    devFlag;
    /*video output device type:enum amba_vout_sink_type.*/
    GADI_U32                    sinkType;
    /*enable csc or not.*/
    GADI_U8                     cscEnable;
    /*enable video or not.*/
    GADI_U8                     videoEnable;
    /*rotate video or not: enum amba_vout_rotate_info*/
    GADI_U8                     rotateEnable;
    /*framebuffer index.*/
    GADI_U8                     fbId;
    /*video output size struct.*/
    GADI_VOUT_VideoSizeT        videoSize;
    /*video output offset struct.*/
    GADI_VOUT_VideoOffsetT      videoOffset;

} GADI_VOUT_OpenParamsT;

/*!
*******************************************************************************
** \brief video output setting parameters struct.
*******************************************************************************
*/
typedef struct
{
    /*video output resolution mode:enum amba_video_mode*/
    GADI_U32                    resoluMode;
    /*video output device type:enum amba_vout_sink_type.*/
    GADI_U32                    sinkType;
} GADI_VOUT_SettingParamsT;


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
** \brief Initialize the ADI video out module.
**
** \return
** - #GADI_OK
** - #GADI_VOUT_ERR_ALREADY_INITIALIZED
** - #GADI_VOUT_ERR_OUT_OF_MEMORY
**
** \sa gadi_vout_exit
**
*******************************************************************************
*/
GADI_ERR gadi_vout_init(void);

/*!
*******************************************************************************
** \brief Shutdown the ADI video output module.
**
** \return
** - #GADI_OK
** - #GADI_VOUT_ERR_NOT_INITIALIZED
**
** \sa gadi_vout_init
**
*******************************************************************************
*/
GADI_ERR gadi_vout_exit(void);

/*!
*******************************************************************************
** \brief Open one ADI video out module instance.
**
** \param[in] openParams a struct pointer of open prameters.
** \param[in] errorCodePtr pointer to return the error code.
**
** \return Return an valid handle of ADI vout module instance.
**
** \sa gadi_vout_close
**
*******************************************************************************
*/
GADI_SYS_HandleT gadi_vout_open(GADI_VOUT_OpenParamsT* openParams, GADI_ERR* errorCodePtr);

/*!
*******************************************************************************
** \brief close one ADI video output module instance.
**
** \param[in]  handle     Valid ADI vin instance handle previously opened by
**                        #gadi_vout_open.
**
** \return
** - #GADI_OK
** - #GADI_VOUT_ERR_BAD_PARAMETER
**
** \sa gadi_vout_open
**
*******************************************************************************
*/
GADI_ERR gadi_vout_close(GADI_SYS_HandleT handle);

/*!
*******************************************************************************
** \brief get video output parameters.
**
** \param[in]  handle     Valid dsp ADI instance handle previously opened by
**                        #gadi_vout_open.
** \param[in]  vinParams   pointer to return the video output parameters struct.
**
** \return
** - #GADI_OK
** - #GADI_VOUT_ERR_NOT_OPEN
** - #GADI_VOUT_ERR_FROM_DRIVER
**
** \sa gadi_vout_open
**
*******************************************************************************
*/
GADI_ERR gadi_vout_get_params(GADI_SYS_HandleT handle, GADI_VOUT_SettingParamsT* voutParams);

/*!
*******************************************************************************
** \brief set video output parameters.
**
** \param[in]  handle     Valid dsp ADI instance handle previously opened by
**                        #gadi_vin_open.
** \param[in]  vinParams   input pointer of video output parameters struct.
**
** \return
** - #GADI_OK
** - #GADI_VOUT_ERR_NOT_OPEN
** - #GADI_VOUT_ERR_FROM_DRIVER
** - #GADI_VOUT_ERR_BAD_PARAMETER
**
** \sa gadi_vout_open
**
*******************************************************************************
*/
GADI_ERR gadi_vout_set_params(GADI_SYS_HandleT handle, GADI_VOUT_SettingParamsT* voutParams);


#ifdef __cplusplus
    }
#endif


#endif /* _ADI_VOUT_H_ */
