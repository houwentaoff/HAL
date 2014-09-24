/*!
*****************************************************************************
** \file        ./adi/inc/adi_priv_dsp.h
**
** \version     $Id: adi_priv_dsp.h 1386 2014-09-12 09:11:22Z fengxuequan $
**
** \brief       adi private dsp API header file.
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#ifndef _ADI_PRIV_DSP_H_
#define _ADI_PRIV_DSP_H_

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
#define GADI_DSP_ERR_BAD_PARAMETER                                          \
                              (GADI_DSP_MODULE_BASE + GADI_ERR_BAD_PARAMETER)
/*! Memory allocation failed. */
#define GADI_DSP_ERR_OUT_OF_MEMORY                                          \
                              (GADI_DSP_MODULE_BASE + GADI_ERR_OUT_OF_MEMORY)
/*! Device already initialised. */
#define GADI_DSP_ERR_ALREADY_INITIALIZED                                    \
                              (GADI_DSP_MODULE_BASE + GADI_ERR_ALREADY_INITIALIZED)
/*! Device not initialised. */
#define GADI_DSP_ERR_NOT_INITIALIZED                                        \
                              (GADI_DSP_MODULE_BASE + GADI_ERR_NOT_INITIALIZED)
/*! Feature or function is not available. */
#define GADI_DSP_ERR_FEATURE_NOT_SUPPORTED                                  \
                              (GADI_DSP_MODULE_BASE + GADI_ERR_FEATURE_NOT_SUPPORTED)
/*! Timeout occured. */
#define GADI_DSP_ERR_TIMEOUT                                                \
                              (GADI_DSP_MODULE_BASE + GADI_ERR_TIMEOUT)
/*! The device is busy, try again later. */
#define GADI_DSP_ERR_DEVICE_BUSY                                            \
                              (GADI_DSP_MODULE_BASE + GADI_ERR_DEVICE_BUSY)
/*! Invalid handle was passed. */
#define GADI_DSP_ERR_INVALID_HANDLE                                         \
                              (GADI_DSP_MODULE_BASE + GADI_ERR_INVALID_HANDLE)
/*! Semaphore could not be created. */
#define GADI_DSP_ERR_SEMAPHORE_CREATE                                       \
                               (GADI_DSP_MODULE_BASE + GADI_ERR_SEMAPHORE_CREATE)
/*! The driver's used version is not supported. */
#define GADI_DSP_ERR_UNSUPPORTED_VERSION                                    \
                               (GADI_DSP_MODULE_BASE + GADI_ERR_UNSUPPORTED_VERSION)
/*! The driver's used version is not supported. */
#define GADI_DSP_ERR_FROM_DRIVER                                            \
                               (GADI_DSP_MODULE_BASE + GADI_ERR_FROM_DRIVER)
/*! The device/handle is not open.. */
#define GADI_DSP_ERR_NOT_OPEN                                               \
                               (GADI_DSP_MODULE_BASE + GADI_ERR_NOT_OPEN)


//*****************************************************************************
//*****************************************************************************
//** Enumerated types
//*****************************************************************************
//*****************************************************************************
/*!
*******************************************************************************
** \brief dsp ucode state.
*******************************************************************************
*/
typedef enum {
    GADI_DSP_UCODE_STATE_UNLOADED = 0,
    GADI_DSP_UCODE_STATE_LOAD,
} GADI_DSP_UCODE_StateEnumT;


/*!
*******************************************************************************
** \brief dsp iav driver state.
*******************************************************************************
*/
typedef enum {
    GADI_DSP_IAV_STATE_IDLE             = 0,
    GADI_DSP_IAV_STATE_PREVIEW          = 1,
    GADI_DSP_IAV_STATE_ENCODING         = 2,
    GADI_DSP_IAV_STATE_STILL_CAPTURE    = 3,
    GADI_DSP_IAV_STATE_DECODING         = 4,
    GADI_DSP_IAV_STATE_TRANSCODING      = 5,
    GADI_DSP_IAV_STATE_DUPLEX           = 6,
    GADI_DSP_IAV_STATE_ENCODE_STARTING  = 7,
    GADI_DSP_IAV_STATE_ENCODE_STOPPING  = 8,
    GADI_DSP_IAV_STATE_PREVIEW_STARTING = 9,
    GADI_DSP_IAV_STATE_INIT             = 0xFF,
} GADI_DSP_IAV_StateEnumt;


//*****************************************************************************
//*****************************************************************************
//** Data Structures
//*****************************************************************************
//*****************************************************************************

/*!
*******************************************************************************
** \brief dsp handl struct.
*******************************************************************************
*/
typedef struct
{
    /*file descriptor of IAV device.*/
    GADI_S32            fdIav;
    /*file descriptor of ucode device.*/
    GADI_S32            fdUcode;
    /*the state of ucode.*/
    GADI_U32            ucodeState;
} GADI_DSP_HandleT;

/*!
*******************************************************************************
** \brief Parameters for getting the ucode version.
*******************************************************************************
*/
typedef struct
{
    /*! ucode version number. */
    GADI_U32    verNum;
    /*! ucode sub version number. */
    GADI_U32    verSubNum;
}GADI_DSP_UCODE_VerParamsT;


/*!
*******************************************************************************
** \brief Parameters for getting the iav driver information.
*******************************************************************************
*/
typedef struct
{
    GADI_U32    arch;
    GADI_U32    model;
    GADI_U32    major;
    GADI_U32    minor;
    GADI_U32    patch;
    GADI_CHAR   description[64];
} GADI_DSP_IAV_InfoParamsT;


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
** \brief Initialize the ADI DSP module.
**
** \return
** - #GADI_OK
** - #GADI_DSP_ERR_ALREADY_INITIALIZED
** - #GADI_DSP_ERR_OUT_OF_MEMORY
**
** \sa gadi_priv_dsp_exit
**
*******************************************************************************
*/
GADI_ERR gadi_priv_dsp_init(void);

/*!
*******************************************************************************
** \brief Shutdown the ADI DSP module.
**
** \return
** - #GADI_OK
** - #GADI_DSP_ERR_NOT_INITIALIZED
**
** \sa gadi_priv_dsp_init
**
*******************************************************************************
*/
GADI_ERR gadi_priv_dsp_exit(void);

/*!
*******************************************************************************
** \brief load the ucode into dsp.
**
** \param[in]  ucodePath  The ucode files' path. It will use the default path
**                        ("/lib/firmware"), when ucodePath set NULL.
**
** \return
** - #GADI_OK
** - #GADI_DSP_ERR_FROM_DRIVER
** - #GADI_DSP_ERR_NOT_INITIALIZED
**
** \sa gadi_priv_dsp_init
**
*******************************************************************************
*/
GADI_ERR gadi_priv_ucode_load(const GADI_CHAR* ucodePath);

/*!
*******************************************************************************
** \brief Get ucode version after loaded the ucode.
**
** \param[in]  ver    pointer to return the struct of version number.
**
** \return
** - #GADI_OK
** - #GADI_DSP_ERR_FROM_DRIVER
** - #GADI_DSP_ERR_BAD_PARAMETER
** - #GADI_DSP_ERR_NOT_INITIALIZED
**
** \sa gadi_priv_ucode_load
**
*******************************************************************************
*/
GADI_ERR gadi_priv_ucode_get_version(GADI_DSP_UCODE_VerParamsT* ver);

/*!
*******************************************************************************
** \brief Get iav driver information.
**
** \param[in]  iavInfo  pointer to return the struct of iav driver information.
**
** \return
** - #GADI_OK
** - #GADI_DSP_ERR_FROM_DRIVER
** - #GADI_DSP_ERR_BAD_PARAMETER
** - #GADI_DSP_ERR_NOT_INITIALIZED
**
** \sa gadi_priv_dsp_open
**
*******************************************************************************
*/
GADI_ERR gadi_priv_iav_get_info(GADI_DSP_IAV_InfoParamsT* iavInfo);

/*!
*******************************************************************************
** \brief Get iav driver state.
**
** \param[in]  iavState  pointer to return the enum of GADI_DSP_IAV_StateEnumt.
**
** \return
** - #GADI_OK
** - #GADI_DSP_ERR_FROM_DRIVER
** - #GADI_DSP_ERR_BAD_PARAMETER
** - #GADI_DSP_ERR_NOT_INITIALIZED
**
** \sa gadi_priv_dsp_open
**
*******************************************************************************
*/
GADI_ERR gadi_priv_iav_get_state(GADI_U32* iavState);

/*!
*******************************************************************************
** \brief iav driver enter idle state.
**
** \return
** - #GADI_OK
** - #GADI_DSP_ERR_FROM_DRIVER
** - #GADI_DSP_ERR_BAD_PARAMETER
** - #GADI_DSP_ERR_NOT_INITIALIZED
**
** \sa gadi_dsp_open
**
*******************************************************************************
*/
GADI_ERR gadi_priv_iav_enter_idle(void);

/*!
*******************************************************************************
** \brief Get iav device's file descriptor.
**
** \param[in]  iavFd  pointer to return the file descriptor.
**
** \return
** - #GADI_OK
** - #GADI_DSP_ERR_BAD_PARAMETER
** - #GADI_DSP_ERR_NOT_INITIALIZED
**
** \sa gadi_dsp_open
**
*******************************************************************************
*/
GADI_ERR gadi_priv_iav_get_fd(GADI_S32* iavFd);


#ifdef __cplusplus
    }
#endif


#endif /* _ADI_PRIV_DSP_H_ */
