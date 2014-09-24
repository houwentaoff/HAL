/*!
*****************************************************************************
** \file        ./adi/inc/adi_priv.h
**
** \version     $Id: adi_priv.h 1386 2014-09-12 09:11:22Z fengxuequan $
**
** \brief       adi private API header.
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#ifndef _ADI_PRIV_H_
#define _ADI_PRIV_H_

#include "stdio.h"
#include "adi_types.h"

#include "adi_vin.h"
#include "adi_vout.h"
#include "adi_priv_dsp.h"

//*****************************************************************************
//*****************************************************************************
//** Defines and Macros
//*****************************************************************************
//*****************************************************************************



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
** \brief video input modul handle struct.
*******************************************************************************
*/
typedef struct
{
    /*file descriptor of IAV device.*/
    GADI_S32                    fdIav;
    /*iav driver state.*/
    GADI_U32                    state;
    /*enum amba_video_mode*/
    GADI_U32                    mode;
    /*input video frame rate.*/
    GADI_U32                    frameRate;
    /*input video source picture width.*/
    GADI_U32                    vinWidth;
    /*input video source picture height.*/
    GADI_U32                    vinHeight;
    /*only for sensor that output YUV data, 0:no-anti; 1:50Hz; 2:60Hz*/
    GADI_S32                    antiFlicker;
    /*AMBA_VIN_SRC_DEV_TYPE_CCD/AMBA_VIN_SRC_DEV_TYPE_DECODER*/
    GADI_S32                    sensorType;
    /*input video mirror mode.*/
    GADI_VIN_MirrorModeParamsT  mirrorMode;
} GADI_VIN_HandleT;

/*!
*******************************************************************************
** \brief video output modul handle struct.
*******************************************************************************
*/
typedef struct
{
    /*file descriptor of IAV device.*/
    GADI_S32                    fdIav;
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

} GADI_VOUT_HandleT;


//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
    }
#endif


#endif /* _ADI_PRIV_H_ */
