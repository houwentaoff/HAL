/*!
*****************************************************************************
** \file            adi/src/adi_gpio.c
**
** \version       $Id: adi_gpio.c 1 2014-08-27 09:24:58Z yuan chuancheng $
**
** \brief          ADI gpio module function
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**                 ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**                 OMMISSIONS
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

#include"adi_sys.h"
#include"adi_types.h"
#include"adi_gpio.h"


//*****************************************************************************
//*****************************************************************************
//** Local Defines
//*****************************************************************************
//*****************************************************************************

/* /sys/class/gpio PATH */
#define GPIO_EXPORT_PATH            "/sys/class/gpio/export"
#define GPIO_ACTIVE_LOW_PATH        "/sys/class/gpio/gpio%d/active_low"
#define GPIO_DIRECTION_PATH         "/sys/class/gpio/gpio%d/direction"
#define GPIO_VALUE_PATH             "/sys/class/gpio/gpio%d/value"
#define GPIO_UNEXPORT_PATH          "/sys/class/gpio/unexport"

#define GPIO_OPERATION_PATH         "/sys/class/gpio/gpio%d"



//*****************************************************************************
//*****************************************************************************
//** Local structures
//*****************************************************************************
//*****************************************************************************

typedef struct
{
    /* file descriptor of export */
    GADI_S32 fdExport;
    /* file descriptor of unexport */
    GADI_S32 fdUnexport;
    /* file descriptor of active_low */
    GADI_S32 fdActiveLow;
    /* file descriptor of direction */
    GADI_S32 fdDirection;
    /* file descriptor of value */
    GADI_S32 fdValue;

    /* GPIO number */
    GADI_U8  gpioNum;
    /* level mode, 1/0 */
    GADI_U8  active_low;
    /* output/input, 1/0 */
    GADI_U8  direction;
    /*high/low level, 1/0*/
    GADI_U8  value;
}GADI_GPIO_HandleT;

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


//*****************************************************************************
//*****************************************************************************
//** Local Functions Declaration
//*****************************************************************************
//*****************************************************************************



//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************

GADI_SYS_HandleT gadi_gpio_open(GADI_ERR *ErrorCode, GADI_GPIO_OpenParam *UserConfig)
{
    GADI_S32 err = 0;
    GADI_CHAR str[3];
    GADI_CHAR buf[50];
    gadi_sys_memset(str, 0, sizeof(str));
    gadi_sys_memset(buf, 0, sizeof(buf));

    GADI_GPIO_HandleT *gpioHandle = NULL;

    /*get memory for gpioHandle*/
    gpioHandle = gadi_sys_malloc(sizeof(GADI_GPIO_HandleT));
    if(gpioHandle == NULL)
    {
        GADI_ERROR("gadi_gpio_open: get memory for gpioHandle failed!\n");
        *ErrorCode = GADI_GPIO_ERR_OUT_OF_MEMORY;
        gadi_sys_free(gpioHandle);
        gpioHandle = NULL;
        return NULL;
    }
    gadi_sys_memset(gpioHandle, 0, sizeof(GADI_GPIO_HandleT));

    /*paramter checkout*/
    if((UserConfig->num_gpio < 0)||(UserConfig->num_gpio > 95))
    {
        GADI_ERROR("The GPIO number out of range.\n");
        *ErrorCode = GADI_GPIO_ERR_BAD_PARAMETER;
        gadi_sys_free(gpioHandle);
        gpioHandle = NULL;
        return NULL;
    }
    if((UserConfig->active_low != 1)&&(UserConfig->active_low != 0))
    {
        GADI_ERROR("The value of active_low is illegal.\n");
        *ErrorCode = GADI_GPIO_ERR_BAD_PARAMETER;
        gadi_sys_free(gpioHandle);
        gpioHandle = NULL;
        return NULL;
    }
    if((UserConfig->direction != 1)&&(UserConfig->direction != 0))
    {
        GADI_ERROR("The value of direction is illegal.\n");
        *ErrorCode = GADI_GPIO_ERR_BAD_PARAMETER;
        gadi_sys_free(gpioHandle);
        gpioHandle = NULL;
        return NULL;
    }
    if((UserConfig->value != 1)&&(UserConfig->value != 0))
    {
        GADI_ERROR("The value is illegal.\n");
        *ErrorCode = GADI_GPIO_ERR_BAD_PARAMETER;
        gadi_sys_free(gpioHandle);
        gpioHandle = NULL;
        return NULL;
    }

    /*get parameter for gpioHandle*/
    gpioHandle->gpioNum    = UserConfig->num_gpio;
    gpioHandle->active_low = UserConfig->active_low;
    gpioHandle->direction  = UserConfig->direction;
    gpioHandle->value      = UserConfig->value;

    /*get file descriptor of GPIO and set*/
    sprintf(str, "%d", gpioHandle->gpioNum);
    if(access(GPIO_EXPORT_PATH, F_OK) != 0)
    {
        GADI_ERROR("The file %s doesn`t exist!(err = %d).\n", GPIO_EXPORT_PATH, err);
        *ErrorCode = GADI_GPIO_ERR_FEATURE_NOT_SUPPORTED;
        gadi_sys_free(gpioHandle);
        gpioHandle = NULL;
        return NULL;
    }
    err = 0;
    sprintf(buf, GPIO_OPERATION_PATH, gpioHandle->gpioNum);
    if(access(buf, F_OK) != 0)
    {
        gpioHandle->fdExport = open(GPIO_EXPORT_PATH, O_WRONLY);
        if(gpioHandle->fdExport < 0)
        {
            GADI_ERROR("Open file %s failed!(gpioHandle->fdExport = %d).\n", GPIO_EXPORT_PATH, gpioHandle->fdExport);
            *ErrorCode = GADI_GPIO_ERR_FROM_DRIVER;
            gadi_sys_free(gpioHandle);
            gpioHandle = NULL;
            return NULL;
        }
        err = write(gpioHandle->fdExport, str, 2);
        if(err < 0)
        {
            GADI_ERROR("export write error!(err = %d).\n", err);
            *ErrorCode = GADI_GPIO_ERR_WRITE_FAILED;
            gadi_sys_free(gpioHandle);
            gpioHandle = NULL;
            return NULL;
        }
        close(gpioHandle->fdExport);
        err = 0;
    }
    else
    {
        GADI_INFO("The file %s exist!.\n", buf);
        GADI_INFO("operation_path_buf = %s\n", buf);
    }
    gadi_sys_memset(str, 0, sizeof(str));
    gadi_sys_memset(buf, 0, sizeof(buf));

    /*get file descriptor of active_low and configure*/
    sprintf(buf, GPIO_ACTIVE_LOW_PATH, gpioHandle->gpioNum);
    gpioHandle->fdActiveLow = open(buf, O_RDWR);
    if(gpioHandle->fdActiveLow < 0)
    {
        GADI_ERROR("Open file %s failed!(gpioHandle->fdActiveLow = %d).\n", buf, gpioHandle->fdActiveLow);
        *ErrorCode = GADI_GPIO_ERR_FROM_DRIVER;
        gadi_sys_free(gpioHandle);
        gpioHandle = NULL;
        return NULL;
    }
    if(gpioHandle->active_low == 1)
        err = write(gpioHandle->fdActiveLow, "1", 1);
    else
        err = write(gpioHandle->fdActiveLow, "0", 1);
    if(err < 0)
    {
        GADI_ERROR("active_low write error(err = %d).\n", err);
        *ErrorCode = GADI_GPIO_ERR_WRITE_FAILED;
        gadi_sys_free(gpioHandle);
        gpioHandle = NULL;
        return NULL;
    }
    close(gpioHandle->fdActiveLow);
    gadi_sys_memset(buf, 0, sizeof(buf));
    err = 0;

    /*get file descriptor of direction and configure*/
    sprintf(buf, GPIO_DIRECTION_PATH, gpioHandle->gpioNum);
    gpioHandle->fdDirection = open(buf, O_RDWR);
    if(gpioHandle->fdDirection < 0)
    {
        GADI_ERROR("Open file %s failed!(gpioHandle->fdDirection = %d).\n", buf, gpioHandle->fdDirection);
        *ErrorCode = GADI_GPIO_ERR_FROM_DRIVER;
        gadi_sys_free(gpioHandle);
        gpioHandle = NULL;
        return NULL;
    }
    if(gpioHandle->direction == 1)
        err = write(gpioHandle->fdDirection, "out", 3);
    else
        err = write(gpioHandle->fdDirection, "in", 2);
    if(err < 0)
    {
        GADI_ERROR("direction write error(err = %d).\n", err);
        *ErrorCode = GADI_GPIO_ERR_WRITE_FAILED;
        gadi_sys_free(gpioHandle);
        gpioHandle = NULL;
        return NULL;
    }
    close(gpioHandle->fdDirection);
    gadi_sys_memset(buf, 0, sizeof(buf));
    err = 0;

    /*get file descriptor of value and configure*/
    if(gpioHandle->direction== 1)
    {
        sprintf(buf, GPIO_VALUE_PATH, gpioHandle->gpioNum);
        gpioHandle->fdValue = open(buf, O_RDWR);
        if(gpioHandle->fdValue < 0)
        {
            GADI_ERROR("Open file %s failed!(gpioHandle->fdValue = %d).\n", buf, gpioHandle->fdValue);
            *ErrorCode = GADI_GPIO_ERR_FROM_DRIVER;
            gadi_sys_free(gpioHandle);
            gpioHandle = NULL;
            return NULL;
        }
        if(gpioHandle->value == 1)
            err = write(gpioHandle->fdValue, "1", 1);
        else
            err = write(gpioHandle->fdValue, "0", 1);
        if(err < 0)
        {
            GADI_ERROR("value write error(err = %d).\n", err);
            *ErrorCode = GADI_GPIO_ERR_WRITE_FAILED;
            gadi_sys_free(gpioHandle);
            gpioHandle = NULL;
            return NULL;
        }
        close(gpioHandle->fdValue);
        gadi_sys_memset(buf, 0, sizeof(buf));
        err = 0;
    }

    *ErrorCode = GADI_OK;
    return (GADI_SYS_HandleT)gpioHandle;

}

GADI_ERR gadi_gpio_close(GADI_SYS_HandleT handle)
{
    GADI_GPIO_HandleT *HandleOperate = (GADI_GPIO_HandleT*)handle;
    if(HandleOperate == NULL)
    {
        return GADI_ERR_BAD_PARAMETER;
    }
    GADI_S32 err = 0;
    GADI_CHAR str[3];
    GADI_CHAR buf[50];
    gadi_sys_memset(buf, 0, sizeof(buf));
    gadi_sys_memset(str, 0, sizeof(str));
    sprintf(str, "%d", HandleOperate->gpioNum);
    if(access(GPIO_UNEXPORT_PATH, F_OK) != 0)
    {
        GADI_ERROR("The file %s doesn`t exist!(err = %d).\n", GPIO_UNEXPORT_PATH, err);
        return GADI_GPIO_ERR_FEATURE_NOT_SUPPORTED;
    }
    sprintf(buf, GPIO_OPERATION_PATH, HandleOperate->gpioNum);
    if(access(buf, F_OK) == 0)
    {
        HandleOperate->fdUnexport = open(GPIO_UNEXPORT_PATH, O_WRONLY);
        if(HandleOperate->fdUnexport < 0)
        {
            GADI_ERROR("Open file %s failed!(HandleOperate->fdUnexport = %d).\n", GPIO_UNEXPORT_PATH, HandleOperate->fdUnexport);
            return GADI_GPIO_ERR_FROM_DRIVER;
        }
        err = write(HandleOperate->fdUnexport, str, 2);
        if(err < 0)
        {
            GADI_ERROR("unexport write error!(err = %d).\n", err);
            return GADI_GPIO_ERR_WRITE_FAILED;
        }
        close(HandleOperate->fdUnexport);
        err = 0;
    }
    else
    {
        GADI_INFO("The file %s doesn`t exist!.\n", buf);
        GADI_INFO("operation_path_buf = %s\n", buf);
        return GADI_GPIO_ERR_FROM_DRIVER;
    }
    gadi_sys_free(HandleOperate);
    HandleOperate = NULL;
    return GADI_OK;
}

GADI_ERR gadi_gpio_set(GADI_SYS_HandleT handle)
{
    GADI_S32 err = 0;
    GADI_CHAR buf[50];
    gadi_sys_memset(buf, 0, sizeof(buf));
    GADI_GPIO_HandleT *HandleOperate = (GADI_GPIO_HandleT *)handle;
    if(HandleOperate == NULL)
    {
        return GADI_ERR_BAD_PARAMETER;
    }
    /*if GPIO is in output mode, value can be set!*/
    if(HandleOperate->direction == 1)
    {
        sprintf(buf, GPIO_VALUE_PATH, HandleOperate->gpioNum);
        HandleOperate->fdValue = open(buf, O_RDWR);
        if(HandleOperate->fdValue < 0)
        {
            GADI_ERROR("Open file %s failed!(HandleOperate->fdValue = %d).\n", buf, HandleOperate->fdValue);
            return GADI_GPIO_ERR_FROM_DRIVER;
        }
        err = write(HandleOperate->fdValue, "1", 1);
        if(err < 0)
        {
            GADI_ERROR("value write error(err = %d).\n", err);
            return GADI_GPIO_ERR_WRITE_FAILED;
        }
    }
    else
    {
        GADI_INFO("value can not be written!\n");
        return GADI_GPIO_ERR_FEATURE_NOT_SUPPORTED;
    }
    close(HandleOperate->fdValue);
    return GADI_OK;
}

GADI_ERR gadi_gpio_clear(GADI_SYS_HandleT handle)
{
    GADI_S32 err = 0;
    GADI_CHAR buf[50];
    gadi_sys_memset(buf, 0, sizeof(buf));
    GADI_GPIO_HandleT *HandleOperate = (GADI_GPIO_HandleT *)handle;
    if(HandleOperate == NULL)
    {
        return GADI_ERR_BAD_PARAMETER;
    }
    /*if GPIO is in output mode, value can be clear!*/
    if(HandleOperate->direction == 1)
    {

        sprintf(buf, GPIO_VALUE_PATH, HandleOperate->gpioNum);
        HandleOperate->fdValue = open(buf, O_RDWR);
        if(HandleOperate->fdValue < 0)
        {
            GADI_ERROR("Open file %s failed!(HandleOperate->fdValue = %d).\n", buf, HandleOperate->fdValue);
            return GADI_GPIO_ERR_FROM_DRIVER;
        }
        err = write(HandleOperate->fdValue, "0", 1);
        if(err < 0)
        {
            GADI_ERROR("value write error(err = %d).\n", err);
            return GADI_GPIO_ERR_WRITE_FAILED;
        }
    }
    else
    {
        GADI_INFO("value can not be written!\n");
        return GADI_GPIO_ERR_FEATURE_NOT_SUPPORTED;
    }
    close(HandleOperate->fdValue);
    return GADI_OK;

}


/******************************* Get GPIO  State ********************************/


GADI_ERR gadi_gpio_read_value(GADI_SYS_HandleT handle, GADI_S32 *value)
{
    GADI_S32 err = 0;
    GADI_CHAR buf[50];
    GADI_CHAR valuestr[2];
    gadi_sys_memset(valuestr, 0, sizeof(valuestr));
    gadi_sys_memset(buf, 0, sizeof(buf));
    GADI_GPIO_HandleT *HandleOperate = (GADI_GPIO_HandleT*)handle;
    if(HandleOperate == NULL)
    {
        return GADI_ERR_BAD_PARAMETER;
    }
    /*if GPIO is in input mode, value can be read!*/
    if(HandleOperate->direction == 0)
    {

        sprintf(buf, GPIO_VALUE_PATH, HandleOperate->gpioNum);
        HandleOperate->fdValue = open(buf, O_RDWR);
        if(HandleOperate->fdValue < 0)
        {
            GADI_ERROR("Open file %s failed!(HandleOperate->fdValue = %d).\n", buf, HandleOperate->fdValue);
            return GADI_GPIO_ERR_FROM_DRIVER;
        }
        err = read(HandleOperate->fdValue, valuestr, 1);
        if(err < 0)
        {
            GADI_ERROR("value read error(err = %d).\n", err);
            return GADI_ERR_READ_FAILED;
        }
    }
    else
    {
        GADI_INFO("value can not be read!\n");
        return GADI_GPIO_ERR_FEATURE_NOT_SUPPORTED;
    }
    *value = atoi(valuestr);
    close(HandleOperate->fdValue);
    return GADI_OK;
}

//*****************************************************************************
//*****************************************************************************
//** Local Functions
//*****************************************************************************
//*****************************************************************************
