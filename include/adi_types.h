/*!
*****************************************************************************
** \file        ./adi/inc/adi_types.h
**
** \version     $Id: adi_types.h 632 2014-08-01 05:31:20Z fengxuequan $
**
** \brief       adapte driver interface type define
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _ADI_TYPES_H_
#define _ADI_TYPES_H_

#include "adi_sys_error.h"

//*****************************************************************************
//*****************************************************************************
//** Defines and Macros
//*****************************************************************************
//*****************************************************************************
/*!
*******************************************************************************
** \brief ADI type define
*******************************************************************************
*/
typedef unsigned char       GADI_U8;         /*!< 8 bit unsigned integer. */
typedef unsigned short      GADI_U16;        /*!< 16 bit unsigned integer. */
typedef unsigned int        GADI_U32;        /*!< 32 bit unsigned integer. */
typedef unsigned long long  GADI_U64;        /*!< 64 bit unsigned integer. */
typedef signed char         GADI_S8;         /*!< 8 bit signed integer. */
typedef signed short        GADI_S16;        /*!< 16 bit signed integer. */
typedef signed int          GADI_S32;        /*!< 32 bit signed integer. */
typedef signed long long    GADI_S64;        /*!< 64 bit unsigned integer. */
typedef signed int          GADI_ERR;        /*!< error code type .*/
typedef unsigned int        GADI_HANDLE;     /*!< 32 bit unsigned integer. */
typedef char                GADI_CHAR;       /*!< char */

/*! common handle type */
typedef void*               GADI_SYS_HandleT;

/*! semaphore handle type */
typedef unsigned int        GADI_SYS_SemHandleT;

/*! thread handle type */
typedef unsigned int        GADI_SYS_ThreadHandleT;


#if defined(__HIGHC__)
    #undef NULL
    #define NULL 0
#else

#ifndef NULL
    #define NULL 0  /*!< NULL define to 0. */
#endif
#endif


/*!
*******************************************************************************
** \brief Common inline key word.
*******************************************************************************
*/
#if defined(__HIGHC__)
#define FINLINE _Inline
#else
#define FINLINE inline
#endif

#if defined(__GNUC__)
#define _Unaligned
#endif

//*****************************************************************************
//*****************************************************************************
//** Enumerated types
//*****************************************************************************
//*****************************************************************************
/*!
*******************************************************************************
** \brief ADI BOOL enum
*******************************************************************************
*/
typedef enum {
    GADI_FALSE = 0,  /*!< Logical false. */
    GADI_TRUE  = 1   /*!< Logical true. */
} GADI_BOOL;


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


#endif /* _ADI_TYPES_H_ */

