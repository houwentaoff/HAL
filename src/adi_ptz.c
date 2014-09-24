/*!
*****************************************************************************
** \file        adi/src/adi_ptz.c
**
** \version     $Id: adi_ptz.c 1312 2014-09-04 10:07:16Z weiqiang $
**
** \brief       ADI PTZ(PELCO-D/P protocol) module function
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
#include <termios.h>
#include <errno.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include <adi_ptz.h>
#include <adi_ptz_pelcod.h>
#include <adi_sys.h>
//*****************************************************************************
//*****************************************************************************
//** Local Defines
//*****************************************************************************
//*****************************************************************************

/* tty device name */
#define GADI_PTZ_TTY_DEV   "/dev/ttyS1"
#define ADI_PTZ_PROTOCOL_CMD_SIZE_MAX  7
#define ADI_PTZ_PROTOCOL_CMD_BUFFER_SIZE  500
#define ADI_PTZ_THREAD_STACKSIZE            2048
#define ADI_PTZ_THREAD_PRIORITY             3
#define ADI_PTZ_THREAD_NAME                 "gadi:ptz"

//*****************************************************************************
//*****************************************************************************
//** Local structures
//*****************************************************************************
//*****************************************************************************
typedef struct
{
   GADI_U32 size;
   GADI_U8 data[ADI_PTZ_PROTOCOL_CMD_SIZE_MAX];
}GADI_PTZ_cmdT;

typedef struct
{
    /*!ptz  protocol commands buffer size*/
    GADI_U32 size;
    /*!ptz  protocol commands*/
    GADI_PTZ_cmdT *cmds;
    /*!read position*/
    GADI_U32 read;
    /*!write position*/
    GADI_U32 write;
    /*!ptz  protocol commands count*/
    GADI_U32 count;
    /*!mutex*/
    pthread_mutex_t mutex;
    /*!condition*/
    pthread_cond_t  cond;
}GADI_PTZ_cmdBufT;

typedef struct
{
    /*!file descriptor of tty device.*/
    GADI_S32            fdTty;
    /*!termios new option*/
    struct termios opt;
    /*!termios old option*/
    struct termios saveOpt;
} GADI_PTZ_HandleT;
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

static GADI_PTZ_HandleT *ptzHandle;

static GADI_SYS_ThreadHandleT    adiPtzThreadHandle = 0;

static GADI_PTZ_TtyDevOptT defaultTtyOpt = {
    2400,
    8,
    'N',
    1
};

GADI_PTZ_cmdBufT *cmdBuf;

//*****************************************************************************
//*****************************************************************************
//** Local Functions Declaration
//*****************************************************************************
//*****************************************************************************
static void ptz_thread_function(void* optArg);
static GADI_ERR ptz_protocol_cmd_buf_init(GADI_PTZ_cmdBufT *cmdBuf, GADI_U32 size);
static void ptz_protocol_cmd_buf_read(GADI_PTZ_cmdBufT *cmdBuf, GADI_PTZ_cmdT *cmd);
static void ptz_protocol_cmd_buf_write(GADI_PTZ_cmdBufT *cmdBuf, GADI_PTZ_cmdT *cmd);
static GADI_BOOL ptz_protocol_cmd_buf_is_empty(GADI_PTZ_cmdBufT *cmdBuf);
#if 0
static GADI_BOOL ptz_protocol_cmd_buf_is_full(GADI_PTZ_cmdBufT *cmdBuf);
#endif
static void ptz_protocol_cmd_buf_del(GADI_PTZ_cmdBufT *cmdBuf);
static GADI_ERR ptz_send_cmd(GADI_PTZ_cmdT *cmd);
static GADI_ERR ptz_feed_cmd(const GADI_PTZ_ProtocolCmdT *pCmd);
static void ptz_flush_cmds();

//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************
GADI_ERR gadi_ptz_init(GADI_PTZ_ControlHandleT  *ptzControlHandle)
{
    GADI_ERR ret = GADI_OK;

    if(ptzHandle != NULL)
    {
        return GADI_PTZ_ERR_ALREADY_INITIALIZED;
    }
    ptzHandle = gadi_sys_malloc(sizeof(GADI_PTZ_HandleT));
    if(ptzHandle == NULL)
    {
        GADI_ERROR("gadi_PTZ_init: ptz handle malloc memory failed!\n");
        return GADI_PTZ_ERR_OUT_OF_MEMORY;
    }
    gadi_sys_memset(ptzHandle, 0, sizeof(GADI_PTZ_HandleT));

    ret = gadi_ptz_pelcod_init(ptzControlHandle);
    if(ret != GADI_OK) {
        GADI_ERROR("gadi_PTZ_init:pelco-d initialize failed!\n");
        goto out_release_handle;
    }
    ptzControlHandle->protocol = ADI_PTZ_PELCO_D;
    ptzControlHandle->ptzFeedCmd = ptz_feed_cmd;
    ptzControlHandle->ptzFlushCmds = ptz_flush_cmds;

    cmdBuf = (GADI_PTZ_cmdBufT *)gadi_sys_malloc(sizeof(GADI_PTZ_cmdBufT));
    if(cmdBuf == NULL)
    {
        GADI_ERROR("gadi_PTZ_init: command buffer malloc memory failed!\n");
        goto out_pelcod_exit;
    }
    gadi_sys_memset(cmdBuf, 0, sizeof(GADI_PTZ_cmdBufT));

    ret = ptz_protocol_cmd_buf_init(cmdBuf, ADI_PTZ_PROTOCOL_CMD_BUFFER_SIZE);
    if(ret != GADI_OK) {
        GADI_ERROR("gadi_PTZ_init:command buffer initialize failed!\n");
        goto out_release_command_buffer;
    }
    ret = gadi_sys_thread_create(
            ptz_thread_function,
            NULL,
            ADI_PTZ_THREAD_PRIORITY,
            ADI_PTZ_THREAD_STACKSIZE,
            ADI_PTZ_THREAD_NAME,
            &adiPtzThreadHandle);
    if(ret != GADI_OK) {
        GADI_ERROR("gadi_PTZ_init:create thread failed!\n");
        goto out_release_command_buffer;
    }
    else {
       return ret;
    }

    return GADI_OK;

out_release_command_buffer:
    gadi_sys_free(cmdBuf);
out_pelcod_exit:
    gadi_ptz_pelcod_exit();
out_release_handle:
    gadi_sys_free(ptzHandle);

    return ret;
}

GADI_SYS_HandleT gadi_ptz_tty_open(GADI_ERR* errorCodePtr)
{
    GADI_PTZ_HandleT* ptz_handle_temp = NULL;

    if(errorCodePtr == NULL)
    {
        return (GADI_SYS_HandleT)ptz_handle_temp;
    }

    if(ptzHandle == NULL)
    {
        *errorCodePtr = GADI_PTZ_ERR_NOT_INITIALIZED;
        return NULL;
    }
    ptz_handle_temp = ptzHandle;

    ptz_handle_temp->fdTty = open(GADI_PTZ_TTY_DEV, O_RDWR | O_NOCTTY |O_NDELAY);
    if (ptz_handle_temp->fdTty < 0) {
        perror("GADI_PTZ_TTY_DEV");
        *errorCodePtr = GADI_PTZ_ERR_FROM_DRIVER;
        return NULL;
    }

    /*Get and save the current options*/
    tcgetattr(ptz_handle_temp->fdTty,&ptz_handle_temp->opt);
    ptz_handle_temp->saveOpt = ptz_handle_temp->opt;

    /*Enable the receiver and set local mode*/
    ptz_handle_temp->opt.c_cflag =  CLOCAL | CREAD;

    /*Set raw mode, echo off, extended input processing off*/
    ptz_handle_temp->opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK |
                    ECHOCTL | ECHOKE | IEXTEN | ISIG);

    /*Raw output*/
    ptz_handle_temp->opt.c_oflag &= ~OPOST;

    /*Does not map NL-CR or CR-NL*/
    ptz_handle_temp->opt.c_iflag &= ~(INLCR | ICRNL | IGNCR);
    ptz_handle_temp->opt.c_oflag &= ~(ONLCR | OCRNL);

    /*No SIGINT on BREAK, default input parity check off, don't
    strip 8th bit on input, software flow control off*/
    ptz_handle_temp->opt.c_iflag &= ~(BRKINT | INPCK| IXON | IXOFF | IXANY | ISTRIP);

    /*No timeout and blocking read until 1 chars received */
    ptz_handle_temp->opt.c_cc[VTIME] = 0;
    ptz_handle_temp->opt.c_cc[VMIN] = 1;

    /*Flushes the input and output queue*/
    tcflush(ptz_handle_temp->fdTty, TCIOFLUSH);
    /*Set the new options*/
    if(tcsetattr(ptz_handle_temp->fdTty, TCSANOW, &ptz_handle_temp->opt) != GADI_OK) {
        perror("Set Serial failed");
        *errorCodePtr = GADI_PTZ_ERR_FROM_DRIVER;
        return NULL;
    }

    return (GADI_SYS_HandleT)ptz_handle_temp;
}

GADI_ERR gadi_ptz_tty_set(GADI_SYS_HandleT handle, GADI_PTZ_TtyDevOptT *ttyOpt)
{
    GADI_PTZ_HandleT* ptz_handle_temp = (GADI_PTZ_HandleT*)handle;
    GADI_PTZ_TtyDevOptT *tty_opt;
    if(ttyOpt != NULL) {
        tty_opt = ttyOpt;
    }
    else {
        tty_opt = &defaultTtyOpt;
    }

    /*Get the current options*/
    tcgetattr(ptz_handle_temp->fdTty,&ptz_handle_temp->opt);

    switch(tty_opt->speed) {
        case 300:
            cfsetispeed(&ptz_handle_temp->opt, B300);
            cfsetospeed(&ptz_handle_temp->opt, B300);
            break;
        case 1200:
            cfsetispeed(&ptz_handle_temp->opt, B1200);
            cfsetospeed(&ptz_handle_temp->opt, B1200);
            break;
        case 2400:
            cfsetispeed(&ptz_handle_temp->opt, B2400);
            cfsetospeed(&ptz_handle_temp->opt, B2400);
            break;
        case 4800:
            cfsetispeed(&ptz_handle_temp->opt, B4800);
            cfsetospeed(&ptz_handle_temp->opt, B4800);
            break;
        case 9600:
            cfsetispeed(&ptz_handle_temp->opt, B9600);
            cfsetospeed(&ptz_handle_temp->opt, B9600);
            break;
        case 19200:
            cfsetispeed(&ptz_handle_temp->opt, B19200);
            cfsetospeed(&ptz_handle_temp->opt, B19200);
            break;
        case 38400:
            cfsetispeed(&ptz_handle_temp->opt, B38400);
            cfsetospeed(&ptz_handle_temp->opt, B38400);
            break;
        case 115200:
            cfsetispeed(&ptz_handle_temp->opt, B115200);
            cfsetospeed(&ptz_handle_temp->opt, B115200);
            break;
        default:
            GADI_ERROR("Unsupported speed\n");
            return GADI_PTZ_ERR_BAD_PARAMETER;
    }

    ptz_handle_temp->opt.c_cflag &= ~CSIZE;
    switch (tty_opt->dataBits) {
        case 7:
            ptz_handle_temp->opt.c_cflag |= CS7;
            break;
        case 8:
            ptz_handle_temp->opt.c_cflag |= CS8;
            break;
        default:
            GADI_ERROR("error: Unsupported data size\n");
            return GADI_PTZ_ERR_BAD_PARAMETER;
    }

    switch (tty_opt->parity) {
        /*No Parity*/
        case 'n':
        case 'N':
            ptz_handle_temp->opt.c_cflag &= ~PARENB;
            ptz_handle_temp->opt.c_iflag &= ~INPCK;
            break;
        /*Odd Parity*/
        case 'o':
        case 'O':
            ptz_handle_temp->opt.c_cflag |= (PARODD|PARENB);
            ptz_handle_temp->opt.c_iflag |= INPCK;
            break;
        /*Even Parity*/
        case 'e':
        case 'E':
            ptz_handle_temp->opt.c_cflag |= PARENB;
            ptz_handle_temp->opt.c_cflag &= ~PARODD;
            ptz_handle_temp->opt.c_iflag |= INPCK;
            break;
        /*Space Parity*/
        case 'S':
        case 's':
            ptz_handle_temp->opt.c_cflag &= ~PARENB;
            ptz_handle_temp->opt.c_cflag &= ~CSTOPB;
            break;
        default:
            GADI_ERROR("Unsupported parity\n");
            return GADI_PTZ_ERR_BAD_PARAMETER;
        }

    switch (tty_opt->stopBits) {
        case 1:
            ptz_handle_temp->opt.c_cflag &= ~CSTOPB;
            break;
        case 2:
            ptz_handle_temp->opt.c_cflag |= CSTOPB;
            break;
        default:
            GADI_ERROR("Unsupported stop bits\n");
            return GADI_PTZ_ERR_BAD_PARAMETER;
    }

    tcflush(ptz_handle_temp->fdTty, TCIOFLUSH);
    if (tcsetattr(ptz_handle_temp->fdTty, TCSANOW,&ptz_handle_temp->opt) != GADI_OK)
    {
        perror("Set Serial failed\n");
        return GADI_PTZ_ERR_FROM_DRIVER;
    }

    return GADI_OK;
}

GADI_ERR gadi_ptz_tty_reset(GADI_SYS_HandleT handle)
{
    GADI_PTZ_HandleT* ptz_handle_temp = (GADI_PTZ_HandleT*)handle;

    if (tcsetattr(ptz_handle_temp->fdTty, TCSANOW, &ptz_handle_temp->saveOpt)
        != GADI_OK) {
        perror("Set Serial failed");
        return GADI_PTZ_ERR_FROM_DRIVER;
    }

    return GADI_OK;
}

void gadi_ptz_tty_close(GADI_SYS_HandleT handle)
{
    GADI_PTZ_HandleT* ptz_handle_temp = (GADI_PTZ_HandleT*)handle;

    close(ptz_handle_temp->fdTty);

    return;
}

GADI_ERR gadi_ptz_exit(GADI_PTZ_ControlHandleT  *ptzControlHandle)
{
    gadi_ptz_pelcod_exit(ptzControlHandle);
    ptz_protocol_cmd_buf_del(cmdBuf);
    gadi_sys_free(ptzControlHandle);
    ptzControlHandle = NULL;
    gadi_sys_thread_destroy(adiPtzThreadHandle);
    if(ptzHandle == NULL)
    {
        return GADI_PTZ_ERR_NOT_INITIALIZED;
    }

    gadi_sys_free(ptzHandle);
    ptzHandle = NULL;

    return GADI_OK;
}

void gadi_ptz_register_protocol(GADI_PTZ_ControlHandleT *ptzControlHandle,
                            const GADI_CHAR *name,
                            ADI_PTZ_ProtocolEnumT protocol,
                            const GADI_PTZ_ProtocolCmdT *pCmd,
                            GADI_PTZ_ProtocolFunsT *pFuns) {

    ptzControlHandle->name[protocol] = name;
    ptzControlHandle->ptzProtocolCmd[protocol] = pCmd;
    ptzControlHandle->ptzProtocolFuns[protocol] = pFuns;

    return;
}

void gadi_ptz_unregister_protocol(GADI_PTZ_ControlHandleT *ptzControlHandle,
                            ADI_PTZ_ProtocolEnumT protocol) {
    ptzControlHandle->name[protocol] = NULL;
    ptzControlHandle->ptzProtocolCmd[protocol] = NULL;
    ptzControlHandle->ptzProtocolFuns[protocol] = NULL;

    return;
}

//*****************************************************************************
//*****************************************************************************
//** Local Functions
//*****************************************************************************
//*****************************************************************************
static void ptz_thread_function(void* optArg)
{
    GADI_PTZ_cmdT cmd;
    GADI_U32 try_again   = 2;
    GADI_U32 i           = 0;

    while(1)
    {
        pthread_mutex_lock(&cmdBuf->mutex);
        while(ptz_protocol_cmd_buf_is_empty(cmdBuf)) {
            pthread_cond_wait(&cmdBuf->cond, &cmdBuf->mutex);
        }
        ptz_protocol_cmd_buf_read(cmdBuf, &cmd);
        GADI_DEBUG("pthread: cmd count[%d]\n", cmdBuf->count);
        pthread_mutex_unlock(&cmdBuf->mutex);

        for(i=0; i < try_again; i++) {
            if(ptz_send_cmd(&cmd) == GADI_OK) {
                break;
            }
        }
        gadi_sys_thread_sleep(250);
    }

}

static GADI_ERR ptz_protocol_cmd_buf_init(GADI_PTZ_cmdBufT *cmdBuf, GADI_U32 size)
{
    GADI_PTZ_cmdBufT *cmd_buf;

    cmd_buf = cmdBuf;

    cmd_buf->size = size;
    cmd_buf->cmds = (GADI_PTZ_cmdT *)gadi_sys_malloc(sizeof(GADI_PTZ_cmdT) * size);
    if(cmd_buf->cmds == NULL)
    {
        GADI_ERROR("ptz_protocol_cmd_buf_init: malloc memory failed!\n");
        return GADI_PTZ_ERR_OUT_OF_MEMORY;
    }
    gadi_sys_memset(cmd_buf->cmds, 0, (sizeof(GADI_PTZ_cmdT) * size));

    pthread_mutex_init(&cmd_buf->mutex, NULL);
    pthread_cond_init(&cmd_buf->cond, NULL);

    return GADI_OK;
}

static void ptz_protocol_cmd_buf_read(GADI_PTZ_cmdBufT *cmdBuf, GADI_PTZ_cmdT *cmd){
    GADI_PTZ_cmdBufT *cmd_buf;

    cmd_buf = cmdBuf;
    *cmd = cmd_buf->cmds[cmd_buf->read];
    cmd_buf->read = (cmd_buf->read + 1) % cmd_buf->size;
    cmd_buf->count--;

    return;
}

static void ptz_protocol_cmd_buf_write(GADI_PTZ_cmdBufT *cmdBuf, GADI_PTZ_cmdT *cmd){
    GADI_PTZ_cmdBufT *cmd_buf;

    cmd_buf = cmdBuf;
    cmd_buf->cmds[cmd_buf->write] = *cmd;
    cmd_buf->write= (cmd_buf->write + 1) % cmd_buf->size;

    if(cmd_buf->count == cmd_buf->size) {
        cmd_buf->read = (cmd_buf->read + 1) % cmd_buf->size;
    }
    else{
        cmd_buf->count++;
    }

    return;
}

static GADI_BOOL ptz_protocol_cmd_buf_is_empty(GADI_PTZ_cmdBufT *cmdBuf)
{
    GADI_PTZ_cmdBufT *cmd_buf;

    cmd_buf = cmdBuf;
    if(cmd_buf->count == 0) {
        return GADI_TRUE;
    }
    else {
        return GADI_FALSE;
    }
}

#if 0
static GADI_BOOL ptz_protocol_cmd_buf_is_full(GADI_PTZ_cmdBufT *cmdBuf)
{
    GADI_PTZ_cmdBufT *cmd_buf;

    cmd_buf = cmdBuf;
    if(cmd_buf->count == 0) {
        return GADI_TRUE;
    }
    else {
        return GADI_FALSE;
    }
}

#endif

static void ptz_protocol_cmd_buf_del(GADI_PTZ_cmdBufT *cmdBuf)
{
    GADI_PTZ_cmdBufT *cmd_buf;

    cmd_buf = cmdBuf;
    gadi_sys_free(cmd_buf->cmds);
    gadi_sys_free(cmd_buf);
    cmd_buf = NULL;

    return;
}

static GADI_ERR ptz_send_cmd(GADI_PTZ_cmdT *cmd)
{
    GADI_U8 *buf;
#ifdef DEBUG
    GADI_U32 i;
#endif

    buf = (GADI_U8*)cmd->data;
#ifdef DEBUG
    printf("send data:\t");
    for(i = 0; i < cmd->size; i++) {
        printf("0x%02x\t", *(buf + i));
    }
    printf("\n");
#endif
    if(ptzHandle->fdTty == 0) {
        GADI_ERROR("tty device not open\n");
        return GADI_PTZ_ERR_NOT_OPEN;
    }
    if(write(ptzHandle->fdTty, buf, cmd->size) == cmd->size) {
        return GADI_OK;
    }
    else {
        perror("send cmd failed");
        tcflush(ptzHandle->fdTty, TCOFLUSH);
        return GADI_PTZ_ERR_FROM_DRIVER;
    }
}

static GADI_ERR ptz_feed_cmd(const GADI_PTZ_ProtocolCmdT *pCmd)
{
    GADI_PTZ_cmdT cmd;

    gadi_sys_memset(&cmd, 0, sizeof(GADI_PTZ_cmdT));
    cmd.size = pCmd->cmdSize;
    gadi_sys_memcpy(cmd.data, pCmd->cmd, cmd.size);
    pthread_mutex_lock(&cmdBuf->mutex);
    if(ptz_protocol_cmd_buf_is_empty(cmdBuf)) {
        ptz_protocol_cmd_buf_write(cmdBuf, &cmd);
        pthread_cond_signal(&cmdBuf->cond);
    }
    else {
        ptz_protocol_cmd_buf_write(cmdBuf, &cmd);
    }
    GADI_DEBUG("feed: cmd count[%d]\n", cmdBuf->count);
    pthread_mutex_unlock(&cmdBuf->mutex);

    return GADI_OK;
}

static void ptz_flush_cmds()
{
    pthread_mutex_lock(&cmdBuf->mutex);
    cmdBuf->read = 0;
    cmdBuf->write = 0;
    cmdBuf->count = 0;
    pthread_mutex_lock(&cmdBuf->mutex);

    return;
}

