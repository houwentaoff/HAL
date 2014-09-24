/*!
*****************************************************************************
** \file        ./adi/inc/adi_sys_error.h
**
** \version     $Id: adi_sys_error.h 13 2014-08-18 07:06:10Z fengxuequan $
**
** \brief       ADI error codes & modules definition.
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _ADI_SYS_ERROR_H_
    #define _ADI_SYS_ERROR_H_

//*****************************************************************************
//*****************************************************************************
//** Defines and Macros
//*****************************************************************************
//*****************************************************************************
/*
********************************************************************************
** Common error codes (reserved from 0 to -99)
**
** Each error code shall be decreased in steps of 1.
********************************************************************************
*/
//! Indication of success.
#define GADI_OK                                        0

//! Bad parameter passed.
#define GADI_ERR_BAD_PARAMETER                        -1

//! Memory allocation failed.
#define GADI_ERR_OUT_OF_MEMORY                        -2

//! Device already initialised.
#define GADI_ERR_ALREADY_INITIALIZED                  -3

//! Device not initialised.
#define GADI_ERR_NOT_INITIALIZED                      -4

//! Feature or function is not available.
#define GADI_ERR_FEATURE_NOT_SUPPORTED                -5

//! Timeout occured.
#define GADI_ERR_TIMEOUT                              -6

//! The device is busy, try again later.
#define GADI_ERR_DEVICE_BUSY                          -7

//! Invalid handle was passed.
#define GADI_ERR_INVALID_HANDLE                       -8

//! Semaphore could not be created.
#define GADI_ERR_SEMAPHORE_CREATE                     -9

//! The driver's used version is not supported.
#define GADI_ERR_UNSUPPORTED_VERSION                 -10

//! Mailqueue coult not be created.
#define GADI_ERR_MAILQUEUE_CREATE                    -11

//! Device/handle is not open.
#define GADI_ERR_NOT_OPEN                            -12

//! Device/handle is already open.
#define GADI_ERR_ALREADY_OPEN                        -13

//! Thread could not be created.
#define GADI_ERR_THREAD_CREATE                       -14

//! Error from driver.
#define GADI_ERR_FROM_DRIVER                         -15

//! Alead running.
#define GADI_ERR_ALREAD_RUNNING                      -16

//! Out of the max size.
#define GADI_ERR_OUT_OF_SIZE                         -17

//! File is written failed.
#define GADI_ERR_WRITE_FAILED                        -18
    
//! File is read failed.
#define GADI_ERR_READ_FAILED                         -19

/*
********************************************************************************
**
** GADI module specific bases (range from -1,000 to -999,000)
**
** Each module base shall be decreased in steps of 1,000.
**
** The module specific error codes shall range from -100 to -999.
********************************************************************************
*/
//! SYS module base
#define GADI_SYS_MODULE_BASE                     -1000

//! audio input module base
#define GADI_AIN_MODULE_BASE                     -2000

//! audio output module base
#define GADI_AOUT_MODULE_BASE                    -3000

//! capture module base
#define GADI_CAP_MODULE_BASE                     -4000

//! face detection module base
#define GADI_FD_MODULE_BASE                      -5000

//! gpio module base
#define GADI_GPIO_MODULE_BASE                    -6000

//! i2c module base
#define GADI_I2C_MODULE_BASE                     -7000

//! image module base
#define GADI_IMG_MODULE_BASE                     -8000

//! motion detection module base
#define GADI_MD_MODULE_BASE                      -9000

//! OSD module base
#define GADI_OSD_MODULE_BASE                     -10000

//! privacy mask module base
#define GADI_PM_MODULE_BASE                      -11000

//! PTZ module base
#define GADI_PTZ_MODULE_BASE                     -12000

//! PWM module base
#define GADI_PWM_MODULE_BASE                     -13000

//! SPI module base
#define GADI_SPI_MODULE_BASE                     -14000

//! video decode module base
#define GADI_VDEC_MODULE_BASE                    -15000

//! video encode module base
#define GADI_VENC_MODULE_BASE                    -16000

//! video input module base
#define GADI_VIN_MODULE_BASE                     -17000

//! video input module base
#define GADI_VIN_MODULE_BASE                     -17000

//! video output module base
#define GADI_VOUT_MODULE_BASE                    -18000

//! dsp output module base
#define GADI_DSP_MODULE_BASE                     -19000


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



#ifdef __cplusplus
}
#endif

#endif /* _ADI_SYS_ERROR_H_ */


