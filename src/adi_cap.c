/*!
*****************************************************************************
** \file        adi/src/adi_cap.c
**
** \version     $Id: adi_cap.c 1 2014-08-11 09:24:58Z fengxuequan $
**
** \brief       ADI caputre(raw data/jpeg) module function
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
#include "adi_cap.h"
#include "adi_priv.h"

//*****************************************************************************
//*****************************************************************************
//** TO BE REMOVED HEADERS !!
//*****************************************************************************
//*****************************************************************************

#include <basetypes.h>
#include "ambas_common.h"

#include "iav_drv.h"
#include "iav_drv_ex.h"

#include "mw_struct.h"
#include "img_struct.h"
#include "img_api.h"

//*****************************************************************************
//*****************************************************************************
//** Local Defines
//*****************************************************************************
//*****************************************************************************

#define CAP_HANDLE_MAX_NUM          1

#define CAP_FILENAME_LENGTH         128

//*****************************************************************************
//*****************************************************************************
//** Local structures
//*****************************************************************************
//*****************************************************************************

typedef struct {

    /* file descriptor of IAV device.*/
    GADI_S32                    fdIav;

    /* iav driver state.*/
    GADI_U32                    state;

    /* file name of the captured files. */
    GADI_CHAR                   fileName[CAP_FILENAME_LENGTH];

    /* file descriptor of file contain capture picture.*/
    GADI_U32                    fdCapFile;

    /* flag to capture raw picture instead of JPEG picture. */
    GADI_U8                     capRaw;

    /* quality factor of JPEG picture. */
    GADI_S32                    quality;

    /* whether this capture handle is in use. */
    GADI_U32                    inUse;

} GADI_CAP_HandleT;


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

static GADI_CAP_HandleT  *capHandleArray = NULL;

static GADI_U32           capInitialized = 0;

//*****************************************************************************
//*****************************************************************************
//** Local Functions Declaration
//*****************************************************************************
//*****************************************************************************
static GADI_CAP_HandleT* cap_allocate_handle(GADI_ERR *errCodePtr);
static void              cap_release_handle(GADI_CAP_HandleT *handlePtr);
static int               cap_save_raw(GADI_CAP_HandleT *handlePtr);
static int               cap_save_jpeg(GADI_CAP_HandleT *handlePtr,
                                            GADI_CAP_StartParamsT *startParamsPtr);

//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************

GADI_ERR gadi_cap_init(void)
{
    if (capInitialized) {
        return GADI_CAP_ERR_ALREADY_INITIALIZED;
    }

    if (capHandleArray) {
        gadi_sys_free(capHandleArray);
        capHandleArray = NULL;
    }

    capHandleArray = (GADI_CAP_HandleT*)gadi_sys_malloc(
                            sizeof(GADI_CAP_HandleT) * CAP_HANDLE_MAX_NUM);

    if (capHandleArray == NULL) {
        return GADI_CAP_ERR_OUT_OF_MEMORY;
    }

    gadi_sys_memset(capHandleArray, 0, sizeof(GADI_CAP_HandleT)*CAP_HANDLE_MAX_NUM);

    capInitialized = 1;

    return GADI_OK;

}

GADI_SYS_HandleT gadi_cap_open(GADI_CAP_OpenParamsT *openParamPtr,
                                     GADI_ERR *errCodePtr)
{
    GADI_CAP_HandleT  *handlePtr = NULL;
    int                state     = 0;
    GADI_S32           iavFd     = 0;
    GADI_ERR           retVal    = GADI_OK;

    retVal = gadi_priv_iav_get_fd(&iavFd);
    if(retVal != GADI_OK)
    {
        GADI_ERROR("gadi_cap_open: gadi_dsp_iav_get_fd: retVal =%d\n", retVal);
        *errCodePtr = retVal;
        return NULL;
    }

    handlePtr = cap_allocate_handle(errCodePtr);

    if (handlePtr == NULL || *errCodePtr == GADI_OK) {

        *errCodePtr = GADI_CAP_ERR_OUT_OF_MEMORY;
        return NULL;
    }

    /* check underlying driver state */
    if (ioctl(iavFd, IAV_IOC_GET_STATE, &state) < 0) {
        *errCodePtr = GADI_CAP_ERR_FROM_DRIVER;
        cap_release_handle(handlePtr);
        return NULL;
    }

    if (state != IAV_STATE_PREVIEW) {
        *errCodePtr = GADI_CAP_ERR_FROM_DRIVER;
        GADI_ERROR("not in right state\n");
        cap_release_handle(handlePtr);
        return NULL;
    }

    /* hold the open parameter */
    handlePtr->fdIav        = iavFd;
    handlePtr->quality      = openParamPtr->quality;

    img_init_still_capture(handlePtr->fdIav, handlePtr->quality);

    return (GADI_SYS_HandleT)handlePtr;

}

GADI_ERR gadi_cap_close(GADI_SYS_HandleT capHandle)
{
    GADI_CAP_HandleT  *handlePtr = (GADI_CAP_HandleT*)capHandle;

    if (ioctl(handlePtr->fdIav, IAV_IOC_LEAVE_STILL_CAPTURE, 0) < 0) {
        perror("IAV_IOC_LEAVE_STILL_CAPTURE");
        return GADI_CAP_ERR_FROM_DRIVER;
    }

    cap_release_handle(handlePtr);

    return GADI_OK;
}

GADI_ERR gadi_cap_start(GADI_SYS_HandleT capHandle,
                       GADI_CAP_StartParamsT *startParamsPtr)
{
    GADI_CAP_HandleT  *handlePtr = (GADI_CAP_HandleT*)capHandle;
    still_cap_info_t   capInfo;

    if (handlePtr == NULL) {
        return GADI_CAP_ERR_INVALID_HANDLE;
    }

    if (startParamsPtr == NULL) {
        return GADI_CAP_ERR_BAD_PARAMETER;
    }

    gadi_sys_memset(&capInfo, 0, sizeof(still_cap_info_t));

    capInfo.capture_num  = startParamsPtr->capNum;
    capInfo.jpeg_w       = startParamsPtr->jpegWidth;
    capInfo.jpeg_h       = startParamsPtr->jpegHeight;
    capInfo.thumb_w      = startParamsPtr->thumbnailWidth;
    capInfo.thumb_h      = startParamsPtr->thumbnialHeight;
    capInfo.need_raw     = startParamsPtr->capRaw;
    capInfo.keep_AR_flag = 1;

    strncpy(handlePtr->fileName, startParamsPtr->fileName, CAP_FILENAME_LENGTH);

    img_start_still_capture(handlePtr->fdIav, &capInfo);

    if (startParamsPtr->capRaw) {
        cap_save_raw(handlePtr);
    }

    if (startParamsPtr->capJpeg || startParamsPtr->capThumbnail) {
        cap_save_jpeg(handlePtr, startParamsPtr);
    }

    return GADI_OK;
}

GADI_ERR gadi_cap_stop(GADI_SYS_HandleT capHandle)
{
    GADI_CAP_HandleT  *handlePtr = (GADI_CAP_HandleT*)capHandle;
    int                ret       = 0;

    ret = img_stop_still_capture(handlePtr->fdIav);
    if (ret < 0) {
        GADI_ERROR("stop still capture failed %d\n", ret);
        return GADI_CAP_ERR_FROM_DRIVER;
    }

    return GADI_OK;
}

GADI_ERR gadi_cap_exit(void)
{
    if (!capInitialized) {
        return GADI_OK;
    }

    if (capHandleArray) {
        gadi_sys_free(capHandleArray);
        capHandleArray = NULL;
    }

    capInitialized = 0;

    return GADI_OK;
}


//*****************************************************************************
//*****************************************************************************
//** Local Functions
//*****************************************************************************
//*****************************************************************************

static GADI_CAP_HandleT* cap_allocate_handle(GADI_ERR *errCodePtr)
{
    GADI_U32            index;
    GADI_CAP_HandleT   *handlePtr = NULL;
    GADI_ERR            error = GADI_OK;

    for (index = 0; index < CAP_HANDLE_MAX_NUM; index++) {
        if (capHandleArray[index].inUse == 0) {
            handlePtr = &capHandleArray[index];
            capHandleArray[index].inUse = 1;
            break;
        }
    }

    if (handlePtr == NULL) {
        error = GADI_CAP_ERR_OUT_OF_MEMORY;
    }

    if (errCodePtr) {
        *errCodePtr = error;
    }

    return handlePtr;
}

static void cap_release_handle(GADI_CAP_HandleT *handlePtr)
{
    if (!handlePtr) {
        return;
    }

    gadi_sys_memset(handlePtr, 0, sizeof(GADI_CAP_HandleT));
}

static int cap_save_raw(GADI_CAP_HandleT *handlePtr)
{
    GADI_S32        fd_raw;
    GADI_CHAR       filename[CAP_FILENAME_LENGTH];
    iav_raw_info_t  raw_info;

    if (ioctl(handlePtr->fdIav, IAV_IOC_READ_RAW_INFO, &raw_info) < 0) {
        perror("IAV_IOC_READ_RAW_INFO");
        return -1;
    }

    GADI_INFO("raw_addr = %p.\n", raw_info.raw_addr);
    GADI_INFO("bit_resolution = %d.\n", raw_info.bit_resolution);
    GADI_INFO("resolution = %dx%d.\n", raw_info.width, raw_info.height);

    sprintf(filename, "%s.raw", handlePtr->fileName);
    if ((fd_raw = open(filename, O_WRONLY | O_CREAT, 0666)) < 0) {
        GADI_INFO("Error opening file [%s]!\n", filename);
        return -1;
    }
    if (write(fd_raw, raw_info.raw_addr, raw_info.width * raw_info.height * 2) < 0) {
        perror("write(5)");
        return -1;
    }

    GADI_INFO("raw picture saved to [%s].\n", filename);
    close(fd_raw);

    return 0;
}

static int cap_save_jpeg(GADI_CAP_HandleT *handlePtr, GADI_CAP_StartParamsT *startParamsPtr)
{
    GADI_S32        fd_jpeg, i, pic_cnt;
    GADI_CHAR       filename[CAP_FILENAME_LENGTH];
    GADI_U32        start_addr, pic_size, bsb_start, bsb_end;

    iav_mmap_info_t mmap_info;
    bs_fifo_info_t  bs_info;

    if (ioctl(handlePtr->fdIav, IAV_IOC_MAP_BSB, &mmap_info) < 0) {
        perror("IAV_IOC_MAP_BSB");
        return -1;
    }
    bsb_start = (u32)mmap_info.addr;
    bsb_end = bsb_start + mmap_info.length;

    for (pic_cnt = 0; pic_cnt < 2*startParamsPtr->capNum; ++pic_cnt) {
        if (ioctl(handlePtr->fdIav, IAV_IOC_READ_BITSTREAM, &bs_info) < 0) {
            perror("IAV_IOC_READ_BITSTREAM");
            return -1;
        }
        if (bs_info.count == 0) {
            GADI_INFO("No bitstream available!\n");
            continue;
        }
        if (((pic_cnt % 2) && (startParamsPtr->capThumbnail== 0)) ||
            ((pic_cnt % 2 == 0) && (startParamsPtr->capJpeg== 0)))
            continue;
        for (i = 0; i < bs_info.count; ++i) {
            sprintf(filename, "%s_%d%03d%s.jpeg", handlePtr->fileName, i,
                (pic_cnt/2), (pic_cnt % 2) ? "_thumb" : "");
            if ((fd_jpeg = open(filename, O_WRONLY | O_CREAT, 0666)) < 0) {
                GADI_INFO("Error opening file [%s]!\n", filename);
                return -1;
            }
            pic_size = (bs_info.desc[i].pic_size + 31) & ~31;
            start_addr = bs_info.desc[i].start_addr;
            if (start_addr + pic_size <= bsb_end) {
                if (write(fd_jpeg, (void *)start_addr, pic_size) < 0) {
                    perror("Save jpeg file!\n");
                    return -1;
                }
            } else {
                u32 size, remain_size;
                size = bsb_end - start_addr;
                remain_size = pic_size - size;
                if (write(fd_jpeg, (void *)start_addr, size) < 0) {
                    perror("Save jpeg file first part!\n");
                    return -1;
                }
                if (write(fd_jpeg, (void *)bsb_start, remain_size) < 0) {
                    perror("Save jpeg file second part!\n");
                    return -1;
                }
            }
            close(fd_jpeg);
        }
        GADI_INFO("Save JPEG %s#: %d.\n", (pic_cnt % 2) ? "Thumbnail " : "",
            pic_cnt/2);
    }

    return 0;
}

