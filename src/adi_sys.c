/*!
*****************************************************************************
** \file        adi/src/adi_sys.c
**
** \version     $Id: adi_sys.c 1 2014-08-11 09:24:58Z fengxuequan $
**
** \brief       ADI system module function
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
#include <sys/time.h>
#include <sys/queue.h>

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <mqueue.h>
#include <time.h>
#include <limits.h>
#include <pthread.h>

#include <semaphore.h>
#include <semaphore.h>

#include "adi_types.h"
#include "adi_sys.h"
#include "adi_priv.h"

//*****************************************************************************
//*****************************************************************************
//** Local Defines
//*****************************************************************************
//*****************************************************************************
#define GADI_SYS_TC_ST_NO_INIT 0
#define GADI_SYS_TC_ST_INIT    1
#define GADI_SYS_THREAD_NAME_SIZE 128


//*****************************************************************************
//*****************************************************************************
//** Local structures
//*****************************************************************************
//*****************************************************************************
/*
*******************************************************************************
** \brief thread with additional data field for this OS.
*******************************************************************************
*/
typedef struct thread_node_t{

    GADI_U32                    stackSize;
    GADI_U32                    priority;
    void*                       function;
    void*                       optArg;
    GADI_CHAR                   optName[GADI_SYS_THREAD_NAME_SIZE];

    pthread_t                   thread_obj;
    pthread_attr_t              thread_attr;
    pthread_mutex_t             thread_mutex;
    pthread_cond_t              thread_cond;

    SLIST_ENTRY(thread_node_t)  entries;

}gadi_sys_linux_thread_t;

typedef struct {
    GADI_U32             inited;
    pthread_mutex_t mutexT;
} gadi_sys_thread_controlT;

SLIST_HEAD(thread_head_t, thread_node_t) threadHead;


//*****************************************************************************
//*****************************************************************************
//** Global Data
//*****************************************************************************
//*****************************************************************************
GADI_U32 adiLogLevel  = GADI_SYS_LOG_LEVLE_INFO;

//*****************************************************************************
//*****************************************************************************
//** Local Data
//*****************************************************************************
//*****************************************************************************
static gadi_sys_thread_controlT gadi_sys_TC = { GADI_SYS_TC_ST_NO_INIT, };


//*****************************************************************************
//*****************************************************************************
//** Local Functions Declaration
//*****************************************************************************
//*****************************************************************************
static void sys_TC_init();
static void sys_TC_lock();
static void sys_TC_unlock();

//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************
GADI_ERR gadi_sys_init(void)
{
    GADI_ERR retVal = GADI_OK;
    GADI_DSP_UCODE_VerParamsT ver;

    SLIST_INIT(&threadHead);
    sys_TC_init();

    /*load ucode.*/
    retVal = gadi_priv_dsp_init();
    if(retVal == GADI_OK)
    {
        retVal = gadi_priv_ucode_load(NULL);
        if(retVal == GADI_OK)
        {
            gadi_priv_ucode_get_version(&ver);
            GADI_INFO("ucode version: %d.%d\n", ver.verNum, ver.verSubNum);
        }
    }

    /*iav driver enter idle state.*/
    if(retVal == GADI_OK)
    {
        retVal = gadi_priv_iav_enter_idle();
    }

    return retVal;
}

GADI_ERR gadi_sys_exit(void)
{
    gadi_priv_dsp_exit();

    return GADI_OK;
}

GADI_ERR gadi_sys_get_log_level(GADI_U32* plogLevel)
{
    if (plogLevel == NULL)
    {
        GADI_ERROR("[gadi_sys_get_log_level] : NULL pointer");
        return GADI_SYS_ERR_BAD_PARAMETER;
    }
    *plogLevel = adiLogLevel;

    return GADI_OK;
}

GADI_ERR gadi_sys_set_log_level(GADI_SYS_LogLevelEnumT logLevel)
{
    if ((logLevel < GADI_SYS_LOG_LEVEL_ERROR) || (logLevel > (GADI_SYS_LOG_LEVEL_NUM -1)))
    {
        GADI_ERROR("Invalid log level, please set it in the range of (0~%d).\n",
                    (GADI_SYS_LOG_LEVEL_NUM - 1));
        return GADI_SYS_ERR_BAD_PARAMETER;
    }
    adiLogLevel = logLevel;

    return GADI_OK;
}

void* gadi_sys_malloc(GADI_U32 size)
{
    void* mem = NULL;

    mem = malloc(size);

    return mem;

}

void gadi_sys_free(void *ptr)
{
    free(ptr);
}

void* gadi_sys_memcpy(void *dest, void *src, GADI_U32 size)
{
    return memcpy(dest, src, size);
}

void* gadi_sys_memset(void *ptr, GADI_U8 c, GADI_U32 n)
{
    return memset(ptr, c, n);
}

GADI_SYS_SemHandleT gadi_sys_sem_create(GADI_U32 initValue)
{
    sem_t* sem = NULL;

    sem = (sem_t*)gadi_sys_malloc(sizeof(sem_t));
    if(sem == NULL) {
        return GADI_SYS_ERR_OUT_OF_MEMORY;
    }
    sem_init(sem, 0, initValue);

    return (GADI_SYS_SemHandleT)sem;
}

GADI_ERR gadi_sys_sem_post(GADI_SYS_SemHandleT sem)
{
    GADI_ERR ret = GADI_OK;

    ret = sem_post((sem_t*)sem);
    if (ret == -1) {
        GADI_ERROR("gadi_sys_sem_post fail %d\n", ret);
        return GADI_SYS_ERR_INVALID_SEMAPHORE_HANDLE;
    }
    return GADI_OK;
}

GADI_ERR gadi_sys_sem_wait(GADI_SYS_SemHandleT sem)
{
    GADI_ERR ret;

    ret = sem_wait((sem_t*)sem);
    if(ret == -1) {
        GADI_ERROR("gadi_sys_sem_wait fail %d\n", ret);
        return GADI_SYS_ERR_INVALID_SEMAPHORE_HANDLE;
    }

    return GADI_OK;
}

GADI_ERR gadi_sys_sem_wait_timeout(GADI_SYS_SemHandleT sem, GADI_U32 msecs)
{
    GADI_ERR  ret;

    struct timespec  tout;
    struct timeval   ttval;

    gettimeofday(&ttval,0);

    tout.tv_nsec = ttval.tv_usec * 1000 + (msecs%1000)*1000000;
    tout.tv_sec  = ttval.tv_sec + msecs/1000;
    ret = sem_timedwait((sem_t*)sem, &tout);

    if (ret == -1) {
        return GADI_SYS_ERR_TIMEOUT;
    }
     return ret;

    return GADI_OK;
}

GADI_ERR gadi_sys_sem_destroy(GADI_SYS_SemHandleT sem)
{
    GADI_ERR  ret;

    ret = sem_destroy((sem_t*)sem);
    if(ret == -1) {
        GADI_ERROR("gadi_sys_sem_destroy fail %d\n", ret);
        return GADI_SYS_ERR_INVALID_SEMAPHORE_HANDLE;
    }

    gadi_sys_free((void*)sem);

    return GADI_OK;
}

void gadi_sys_multi_task_start(void)
{
    while (1)
    {
        sleep(1000);
    }
}

GADI_ERR gadi_sys_thread_create(GADI_SYS_ThreadFunctionT func, void *arg,
                            GADI_S32 priority, GADI_S32 ssize, const char *name,
                            GADI_SYS_ThreadHandleT *thread)
{
    GADI_U32 ret;
    gadi_sys_linux_thread_t *linux_thread;

    //////////////
    sys_TC_lock();
    //////////////

    linux_thread = gadi_sys_malloc(sizeof( gadi_sys_linux_thread_t));
    if(linux_thread == NULL){

        ////////////////
        sys_TC_unlock();
        ////////////////

        return GADI_ERR_OUT_OF_MEMORY;
    }

    ssize *= 1024;

    linux_thread->stackSize = (ssize > PTHREAD_STACK_MIN) ? ssize : PTHREAD_STACK_MIN;
    linux_thread->priority  = priority;
    linux_thread->function  = func;
    linux_thread->optArg    = arg;
    memset(linux_thread->optName, 0, sizeof(linux_thread->optName)-1);
    if (name != NULL){
        strncpy(linux_thread->optName, name, sizeof(linux_thread->optName)-2);
    }

    pthread_mutex_init(&linux_thread->thread_mutex, NULL);
    pthread_cond_init (&linux_thread->thread_cond,  NULL );

    ret = pthread_attr_init(&linux_thread->thread_attr);
    if (ret != 0) {
        GADI_ERROR("ERROR: pthread_attr_init %d\n", ret);
        goto GADI_SYS_CreateThread_error;
    }

    if(ssize != 0) {
    /*set stack size*/
    ret = pthread_attr_setstacksize( &linux_thread->thread_attr, linux_thread->stackSize );
        if ( ret != 0 ) {
            pthread_attr_destroy( &linux_thread->thread_attr );
            GADI_ERROR("ERROR: pthread_attr_setstacksize %d\n", ret );
            goto GADI_SYS_CreateThread_error;
        }
    }

    ret = pthread_create( &linux_thread->thread_obj, &linux_thread->thread_attr,
                          (void *)linux_thread->function, linux_thread->optArg );
    if ( ret != 0 ) {
        pthread_attr_destroy( &linux_thread->thread_attr );
        GADI_ERROR("ERROR: pthread_create %d\n", ret );
        goto GADI_SYS_CreateThread_error;
    }
    if (thread) {
        *thread = (GADI_SYS_ThreadHandleT)linux_thread;
    }

    SLIST_INSERT_HEAD(&threadHead, linux_thread, entries);

    GADI_INFO("%s thread create OK\n", linux_thread->optName);

    ////////////////
    sys_TC_unlock();
    ////////////////

    return GADI_OK;

GADI_SYS_CreateThread_error:
    pthread_mutex_destroy( &linux_thread->thread_mutex );
    gadi_sys_free(linux_thread);

    GADI_ERROR("FAPEX_SYS_CreateThread() error\n" );

    sys_TC_unlock();
    return GADI_ERR_THREAD_CREATE;

}

GADI_ERR gadi_sys_thread_destroy(GADI_SYS_ThreadHandleT thread)
{
    GADI_U32 ret;
    gadi_sys_linux_thread_t *linux_thread = (gadi_sys_linux_thread_t*)thread;

    if ( linux_thread == NULL )
    {
        GADI_ERROR("ERROR: work_thread=%x\n", (GADI_SYS_ThreadHandleT)linux_thread );
        return GADI_ERR_THREAD_CREATE;
    }

    sys_TC_lock();

    ret = pthread_cancel( linux_thread->thread_obj );
    if (ret != 0)
    {
       GADI_ERROR("FAPEX_SYS_DestroyThread() error %d\n", ret );
       sys_TC_unlock();
       return GADI_ERR_BAD_PARAMETER;
    }

    pthread_attr_destroy (&linux_thread->thread_attr);
    pthread_mutex_destroy(&linux_thread->thread_mutex);
    pthread_cond_destroy (&linux_thread->thread_cond );

    SLIST_REMOVE(&threadHead, linux_thread, thread_node_t, entries);

    gadi_sys_free(linux_thread);

    sys_TC_unlock();

    return GADI_OK;
}

void gadi_sys_thread_self_destroy(void)
{
    gadi_sys_linux_thread_t *handlePtr = NULL;

    pthread_t thread = pthread_self();

    SLIST_FOREACH(handlePtr, &threadHead, entries)
        if (handlePtr->thread_obj == thread) break;

    if (handlePtr == NULL)
        return;

    pthread_attr_destroy (&handlePtr->thread_attr);
    pthread_mutex_destroy(&handlePtr->thread_mutex);
    pthread_cond_destroy (&handlePtr->thread_cond );

    pthread_exit(NULL);
}


void gadi_sys_thread_sleep(GADI_U32 msecs)
{
    usleep(1000*msecs);
}

void gadi_sys_thread_suspend(GADI_SYS_ThreadHandleT thread)
{
    gadi_sys_linux_thread_t *handlePtr = (gadi_sys_linux_thread_t*)thread;
    int                      ret;

    if( handlePtr == NULL ) {
        GADI_ERROR("gadi_sys_thread_suspend() error\n");
        return;
    }

    ret = pthread_mutex_lock( &handlePtr->thread_mutex );
    if(ret == 0) {
        ret = pthread_cond_wait( &handlePtr->thread_cond, &handlePtr->thread_mutex);
        if(ret != 0) {
            GADI_ERROR("pthread_cond_wait() error %d\n", ret);
        }
        ret = pthread_mutex_unlock( &handlePtr->thread_mutex );
    }

    if(ret == 0)  {
        return;
    }
    else {
        GADI_ERROR("gadi_sys_thread_suspend() error %d\n", ret);
        return;
    }
}

void gadi_sys_thread_self_suspend(void)
{
    gadi_sys_linux_thread_t *handlePtr = NULL;

    pthread_t thread = pthread_self();

    SLIST_FOREACH(handlePtr, &threadHead, entries)
        if (handlePtr->thread_obj == thread) break;

    if (handlePtr == NULL) {
        GADI_ERROR("this thread has not been registered to ADI\n");
        return;
    }

    gadi_sys_thread_suspend((GADI_SYS_ThreadHandleT)handlePtr);
}

void gadi_sys_thread_resume(GADI_SYS_ThreadHandleT thread)
{
    gadi_sys_linux_thread_t *work_thread = (gadi_sys_linux_thread_t*)thread;
    int                      ret;

    if( work_thread == NULL )
    {
        GADI_ERROR("gadi_sys_thread_resume() error\n");
        return;
    }

    ret = pthread_mutex_lock( &work_thread->thread_mutex );
    if(ret == 0) {
        ret = pthread_cond_signal( &work_thread->thread_cond );
        if(ret != 0) {
            GADI_ERROR("pthread_cond_signal() error %d\n", ret);
        }
        ret = pthread_mutex_unlock( &work_thread->thread_mutex );
    }

    if(ret == 0)  {
        return;
    }
    else {
        GADI_ERROR("gadi_sys_thread_resume() error %d\n", ret);
        return;
    }
}

void gadi_sys_thread_statistics(void)
{
    GADI_INFO("gadi_sys_thread_statistics() not implemented yet\n");
}

//*****************************************************************************
//*****************************************************************************
//** Local Functions
//*****************************************************************************
//*****************************************************************************
static void sys_TC_init(void)
{
    if ( gadi_sys_TC.inited != GADI_SYS_TC_ST_INIT )
    {
        gadi_sys_TC.inited = GADI_SYS_TC_ST_INIT;
        pthread_mutex_init( &(gadi_sys_TC.mutexT), NULL );
    }
}

static void sys_TC_lock(void)
{
    pthread_mutex_lock(&(gadi_sys_TC.mutexT));
}

static void sys_TC_unlock(void)
{
     pthread_mutex_unlock(&(gadi_sys_TC.mutexT));
}


