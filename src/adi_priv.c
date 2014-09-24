/*!
*****************************************************************************
** \file        adi/src/adi_priv.c
**
** \version     $Id: adi_priv.c 1 2014-08-11 09:24:58Z fengxuequan $
**
** \brief       ADI private API.
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
#include <sys/mman.h>

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "adi_types.h"
#include "adi_sys.h"
#include "adi_priv.h"

#include <basetypes.h>
#include "iav_drv.h"

//*****************************************************************************
//*****************************************************************************
//** Local Defines
//*****************************************************************************
//*****************************************************************************
/* DSP device name */
#define GADI_IAV_DEV            "/dev/iav"
#define GADI_UCODE_DEV          "/dev/ucode"
#define GADI_UCODE_DEFAULT_PATH "/lib/firmware"


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
static GADI_DSP_HandleT* dspHandle = NULL;


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
GADI_ERR gadi_priv_dsp_init(void)
{
    if(dspHandle != NULL)
    {
        return GADI_DSP_ERR_ALREADY_INITIALIZED;
    }
    dspHandle = gadi_sys_malloc(sizeof(GADI_DSP_HandleT));
    if(dspHandle == NULL)
    {
        GADI_ERROR("gadi_dsp_init: malloc memory failed!\n");
        return GADI_DSP_ERR_OUT_OF_MEMORY;
    }

    gadi_sys_memset(dspHandle, 0, sizeof(GADI_DSP_HandleT));

    /*open ucode dev*/
    if ((dspHandle->fdUcode = open(GADI_UCODE_DEV, O_RDWR, 0)) < 0)
    {
        perror("/dev/ucode");
        gadi_sys_free(dspHandle);
        dspHandle = NULL;
        return GADI_DSP_ERR_FROM_DRIVER;
    }

    /*open iav dev*/
    if ((dspHandle->fdIav = open(GADI_IAV_DEV, O_RDWR, 0)) < 0)
    {
        perror("/dev/iav");
        close(dspHandle->fdUcode);
        gadi_sys_free(dspHandle);
        dspHandle = NULL;
        return GADI_DSP_ERR_FROM_DRIVER;
    }

    return GADI_OK;
}

GADI_ERR gadi_priv_dsp_exit(void)
{
    if(dspHandle == NULL)
    {
        return GADI_DSP_ERR_NOT_INITIALIZED;
    }

    if(dspHandle->fdUcode > 0)
    {
        close(dspHandle->fdUcode);
    }
    if(dspHandle->fdIav > 0)
    {
        close(dspHandle->fdIav);
    }

    gadi_sys_free(dspHandle);
    dspHandle = NULL;
    return GADI_OK;
}

GADI_ERR gadi_priv_ucode_load(const GADI_CHAR* ucodePath)
{
    GADI_U32 i;
    GADI_U32 fileLength;
    GADI_U8* ucodeMem;
    GADI_U8* addr;
    GADI_CHAR filename[256];
    FILE *fp;
    ucode_load_info_t info;
    GADI_DSP_HandleT* dspHandleTemp = dspHandle;

    if((dspHandleTemp == NULL) || (dspHandleTemp->fdUcode <= 0))
    {
        return GADI_DSP_ERR_NOT_INITIALIZED;
    }

    if (ucodePath == NULL)
    {
        ucodePath = GADI_UCODE_DEFAULT_PATH;
    }

    if (ioctl(dspHandleTemp->fdUcode, IAV_IOC_GET_UCODE_INFO, &info) < 0)
    {
        perror("IAV_IOC_GET_UCODE_INFO");
        return GADI_DSP_ERR_FROM_DRIVER;
    }

    GADI_INFO("map_size = 0x%08x \n", (GADI_U32)info.map_size);
    GADI_INFO("nr_item = %d\n", info.nr_item);
    for (i = 0; i < info.nr_item; i++)
    {
        GADI_INFO("addr_offset = 0x%08X ", (GADI_U32)info.items[i].addr_offset);
        GADI_INFO("filename = %s\n", info.items[i].filename);
    }

    ucodeMem = (GADI_U8 *)mmap(NULL, info.map_size, PROT_READ | PROT_WRITE, MAP_SHARED,
                               dspHandleTemp->fdUcode, 0);
    if ((GADI_S32)ucodeMem == -1)
    {
        perror("mmap");
        return GADI_DSP_ERR_FROM_DRIVER;
    }
    GADI_INFO("mmap returns 0x%x\n", (unsigned)ucodeMem);

    for (i = 0; i < info.nr_item; i++)
    {
        addr = ucodeMem + info.items[i].addr_offset;
        sprintf(filename, "%s/%s", ucodePath, info.items[i].filename);

        GADI_INFO("loading %s...", filename);
        if ((fp = fopen(filename, "rb")) == NULL)
        {
            perror(filename);
            return GADI_DSP_ERR_FROM_DRIVER;
        }

        if (fseek(fp, 0, SEEK_END) < 0)
        {
            perror("SEEK_END");
            return GADI_DSP_ERR_FROM_DRIVER;
        }
        fileLength = ftell(fp);
        if (fseek(fp, 0, SEEK_SET) < 0)
        {
            perror("SEEK_SET");
            return GADI_DSP_ERR_FROM_DRIVER;
        }

        GADI_INFO("addr = 0x%x, size = 0x%x\n", (GADI_U32)addr, fileLength);

        if (fread(addr, 1, fileLength, fp) != fileLength)
        {
            perror("fread");
            return GADI_DSP_ERR_FROM_DRIVER;
        }

        fclose(fp);
    }

    if (ioctl(dspHandleTemp->fdUcode, IAV_IOC_UPDATE_UCODE, 0) < 0)
    {
        perror("IAV_IOC_UPDATE_UCODE");
        return GADI_DSP_ERR_FROM_DRIVER;
    }

    if (munmap(ucodeMem, info.map_size) < 0)
    {
        perror("munmap");
    }

    dspHandleTemp->ucodeState = GADI_DSP_UCODE_STATE_LOAD;

    return GADI_OK;
}

GADI_ERR gadi_priv_ucode_get_version(GADI_DSP_UCODE_VerParamsT* ver)
{
    ucode_version_t version;
    GADI_DSP_HandleT* dspHandleTemp = dspHandle;

    if((dspHandleTemp == NULL) || (dspHandleTemp->fdUcode <= 0))
    {
        return GADI_DSP_ERR_NOT_INITIALIZED;
    }

    if(ver == NULL)
    {
        return GADI_DSP_ERR_BAD_PARAMETER;
    }

    if(dspHandleTemp->ucodeState != GADI_DSP_UCODE_STATE_LOAD)
    {
        return GADI_DSP_ERR_NOT_OPEN;
    }

    if (ioctl(dspHandleTemp->fdUcode, IAV_IOC_GET_UCODE_VERSION, &version) < 0)
    {
        perror("IAV_IOC_GET_UCODE_VERSION");
        return GADI_DSP_ERR_FROM_DRIVER;
    }

    ver->verNum    = version.edition_num;
    ver->verSubNum = version.edition_ver;

    GADI_INFO("===============================\n");
    GADI_INFO("ucode version = %d/%d/%d %d.%d\n",
               version.year, version.month, version.day,
               version.edition_num, version.edition_ver);
    GADI_INFO("===============================\n");

    return GADI_OK;
}

GADI_ERR gadi_priv_iav_get_info(GADI_DSP_IAV_InfoParamsT* iavInfo)
{
    iav_driver_info_t drvInfo;
    GADI_DSP_HandleT* dspHandleTemp = dspHandle;

    if((dspHandleTemp == NULL) || (dspHandleTemp->fdIav <= 0))
    {
        return GADI_DSP_ERR_NOT_INITIALIZED;
    }

    if(iavInfo == NULL)
    {
        return GADI_DSP_ERR_BAD_PARAMETER;
    }

    if (ioctl(dspHandleTemp->fdIav, IAV_IOC_GET_DRIVER_INFO, &drvInfo) < 0)
    {
        perror("IAV_IOC_GET_DRIVER_INFO");
        return GADI_DSP_ERR_FROM_DRIVER;
    }

    iavInfo->arch  = drvInfo.arch;
    iavInfo->model = drvInfo.model;
    iavInfo->major = drvInfo.major;
    iavInfo->minor = drvInfo.minor;
    strncpy(iavInfo->description, drvInfo.description, 63);

    GADI_INFO("======= Driver Info ========\n");
    GADI_INFO("arch = %d\n",  iavInfo->arch);
    GADI_INFO("model = %d\n", iavInfo->model);
    GADI_INFO("major = %d\n", iavInfo->major);
    GADI_INFO("minor = %d\n", iavInfo->minor);
    GADI_INFO("description = %s\n", iavInfo->description);
    GADI_INFO("======= Driver Info ========\n");

    return GADI_OK;
}


GADI_ERR gadi_priv_iav_get_state(GADI_U32* iavState)
{
    GADI_DSP_HandleT* dspHandleTemp = dspHandle;

    if((dspHandleTemp == NULL) || (dspHandleTemp->fdIav <= 0))
    {
        return GADI_DSP_ERR_NOT_INITIALIZED;
    }

    if(iavState == NULL)
    {
        return GADI_DSP_ERR_BAD_PARAMETER;
    }

    if (ioctl(dspHandleTemp->fdIav, IAV_IOC_GET_STATE, iavState) < 0)
    {
        perror("IAV_IOC_GET_STATE");
        return GADI_DSP_ERR_FROM_DRIVER;
    }

    return GADI_OK;
}

GADI_ERR gadi_priv_iav_enter_idle(void)
{
    GADI_U32 state = 0;
    GADI_DSP_HandleT* dspHandleTemp = dspHandle;

    if((dspHandleTemp == NULL) || (dspHandleTemp->fdIav <= 0))
    {
        return GADI_DSP_ERR_NOT_INITIALIZED;
    }

    if (ioctl(dspHandleTemp->fdIav, IAV_IOC_GET_STATE, &state) < 0)
    {
        perror("IAV_IOC_GET_STATE");
        return GADI_DSP_ERR_FROM_DRIVER;
    }
    if ((state != GADI_DSP_IAV_STATE_ENCODING) &&
        (state != GADI_DSP_IAV_STATE_STILL_CAPTURE))
    {
        if (ioctl(dspHandleTemp->fdIav, IAV_IOC_ENTER_IDLE, 0) < 0)
        {
            perror("IAV_IOC_ENTER_IDLE");
            return GADI_DSP_ERR_FROM_DRIVER;
        }
    }
    else
    {
        return GADI_DSP_ERR_FROM_DRIVER;
    }

    return GADI_OK;
}

GADI_ERR gadi_priv_iav_get_fd(GADI_S32* iavFd)
{
    GADI_DSP_HandleT* dspHandleTemp = dspHandle;

    if((dspHandleTemp == NULL) || (dspHandleTemp->fdIav <= 0))
    {
        return GADI_DSP_ERR_NOT_INITIALIZED;
    }

    if(iavFd == NULL)
    {
        return GADI_DSP_ERR_BAD_PARAMETER;
    }

    *iavFd = dspHandleTemp->fdIav;

    return GADI_OK;
}

//*****************************************************************************
//*****************************************************************************
//** Local Functions
//*****************************************************************************
//*****************************************************************************

