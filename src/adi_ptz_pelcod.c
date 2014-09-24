/*!
*****************************************************************************
** \file        adi/src/adi_ptz_pelcod.c
**
** \version     $Id: adi_ptz_pelcod.c 1312 2014-09-04 10:07:16Z weiqiang $
**
** \brief       ADI PTZ PELCO-D protocol
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

#include "adi_sys.h"

#include <adi_ptz.h>
#include <adi_ptz_pelcod.h>

//*****************************************************************************
//*****************************************************************************
//** Local Defines
//*****************************************************************************
//*****************************************************************************
#define GADI_PTZ_PELCOD_SYNCH      0xFF;
#define GADI_PTZ_PELCOD_DEFAULT_ADDR       0x01;

/*command*/
#define GADI_PTZ_PELCOD_STOP 0x0000
#define GADI_PTZ_PELCOD_AUTO_SCAN 0x0090
#define GADI_PTZ_PELCOD_PAN_RIGHT 0x0002
#define GADI_PTZ_PELCOD_PAN_LEFT 0x0004
#define GADI_PTZ_PELCOD_TILT_UP 0x0008
#define GADI_PTZ_PELCOD_TILT_DOWN 0x0010
#define GADI_PTZ_PELCOD_PANTILT_RIGHTUP 0x000a
#define GADI_PTZ_PELCOD_PANTILT_RIGHTDOWN 0x0012
#define GADI_PTZ_PELCOD_PANTILT_LEFTUP 0x000c
#define GADI_PTZ_PELCOD_PANTILT_LEFTDOWN 0x0014
#define GADI_PTZ_PELCOD_ZOOM_TELE 0x0020
#define GADI_PTZ_PELCOD_ZOOM_WIDE 0x0040
#define GADI_PTZ_PELCOD_FOCUS_FAR 0x0080
#define GADI_PTZ_PELCOD_FOCUS_NEAR 0x0100
#define GADI_PTZ_PELCOD_IRIS_OPEN 0x0200
#define GADI_PTZ_PELCOD_IRIS_CLOSE 0x0400


#define GADI_PTZ_PELCOD_SET_PRESET 0x0003
#define GADI_PTZ_PELCOD_CLEAR_PRESET 0x0005
#define GADI_PTZ_PELCOD_GOTO_PRESET 0x0007

/*Pan speed range*/
#define GADI_PTZ_PAN_SPEED_MIN 0x00
#define GADI_PTZ_PAN_SPEED_MAX 0x3F

/*Tilt speed range*/
#define GADI_PTZ_TILT_SPEED_MIN    0x00
#define GADI_PTZ_TILT_SPEED_MAX    0x3F

//*****************************************************************************
//*****************************************************************************
//** Local structures
//*****************************************************************************
//*****************************************************************************
typedef struct {
    /*! synch Byte*/
    GADI_U8     synch;
    /*! Address*/
    GADI_U8     addr;
    /*! Command 1*/
    GADI_U8     cmd1;
    /*! Command 2*/
    GADI_U8     cmd2;
    /*! Data 1 */
    GADI_U8     data1;
    /*! Data 2 */
    GADI_U8     data2;
    /*! CheckSum*/
    GADI_U8     checkSum;
} GADI_PTZ_PelcodMsgT;

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
static const GADI_CHAR *protocolName = "pelco-d";
static ADI_PTZ_ProtocolEnumT protocol;
static GADI_PTZ_PelcodMsgT pelcodMsg;
static GADI_PTZ_ProtocolCmdT *ptzPelcodCmd;
static GADI_PTZ_ProtocolFunsT *ptzPelcodFuns;

//*****************************************************************************
//*****************************************************************************
//** Local Functions Declaration
//*****************************************************************************
//*****************************************************************************
static GADI_U8 get_cmd1(GADI_U16 pelcodCmd);
static GADI_U8 get_cmd2(GADI_U16 pelcodCmd);
static void ptz_pelcod_set_message(GADI_U8 cmd1, GADI_U8 cmd2,
                                GADI_U8 data1, GADI_U8 data2);
static void ptz_pelcod_set_address(GADI_U8 address);
static void ptz_pelcod_stop();
static void ptz_pelcod_autoScan();
static void ptz_pelcod_panRight(GADI_U8 panSpeed);
static void ptz_pelcod_panLeft(GADI_U8 panSpeed);
static void ptz_pelcod_tiltUp(GADI_U8 tiltSpeed);
static void ptz_pelcod_tiltDown(GADI_U8 tiltSpeed);
static void ptz_pelcod_panTilt_rightUp(GADI_U8 panSpeed,
                                    GADI_U8 tiltSpeed);
static void ptz_pelcod_panTilt_rightDown(GADI_U8 panSpeed, GADI_U8 tiltSpeed);
static void ptz_pelcod_panTilt_leftUp(GADI_U8 panSpeed, GADI_U8 tiltSpeed);
static void ptz_pelcod_panTilt_leftDown(GADI_U8 panSpeed, GADI_U8 tiltSpeed);
static void ptz_pelcod_zoomTele();
static void ptz_pelcod_zoomWide();
static void ptz_pelcod_focusFar();
static void ptz_pelcod_focusNear();
static void ptz_pelcod_irisOpen();
static void ptz_pelcod_irisClose();
static void ptz_pelcod_setPreset(GADI_U8 presetNum);
static void ptz_pelcod_clearPreset(GADI_U8 presetNum);
static void ptz_pelcod_goToPreset(GADI_U8 presetNum);
//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************
GADI_ERR gadi_ptz_pelcod_init(GADI_PTZ_ControlHandleT *ptzControlHandle)
{
    protocol = ADI_PTZ_PELCO_D;

    pelcodMsg.synch = GADI_PTZ_PELCOD_SYNCH;
    pelcodMsg.addr  = GADI_PTZ_PELCOD_DEFAULT_ADDR;

    ptzPelcodCmd = gadi_sys_malloc(sizeof(GADI_PTZ_ProtocolCmdT));
    if(ptzPelcodCmd == NULL)
    {
        GADI_ERROR("gadi_ptz_pelcod_init: malloc memory failed!\n");
        return GADI_PTZ_ERR_OUT_OF_MEMORY;
    }
    gadi_sys_memset(ptzPelcodCmd, 0, sizeof(GADI_PTZ_ProtocolCmdT));
    ptzPelcodCmd->cmd = (void*)&pelcodMsg;
    ptzPelcodCmd->cmdSize = sizeof(GADI_PTZ_PelcodMsgT);

    ptzPelcodFuns = gadi_sys_malloc(sizeof(GADI_PTZ_ProtocolFunsT));
    if(ptzPelcodFuns == NULL)
    {
        GADI_ERROR("gadi_ptz_pelcod_init: malloc memory failed!\n");
        gadi_sys_free(ptzPelcodCmd);
        return GADI_PTZ_ERR_OUT_OF_MEMORY;
    }
    gadi_sys_memset(ptzPelcodFuns, 0, sizeof(GADI_PTZ_ProtocolFunsT));
    ptzPelcodFuns->ptzStop = ptz_pelcod_stop;
    ptzPelcodFuns->ptzSetAddress = ptz_pelcod_set_address;
    ptzPelcodFuns->ptzPanLeft = ptz_pelcod_panLeft;
    ptzPelcodFuns->ptzPanRight = ptz_pelcod_panRight;
    ptzPelcodFuns->ptzTiltUp = ptz_pelcod_tiltUp;
    ptzPelcodFuns->ptzTiltDown = ptz_pelcod_tiltDown;
    ptzPelcodFuns->ptzPanTiltRightUp = ptz_pelcod_panTilt_rightUp;
    ptzPelcodFuns->ptzPanTiltRightDown = ptz_pelcod_panTilt_rightDown;
    ptzPelcodFuns->ptzPanTiltLeftUp = ptz_pelcod_panTilt_leftUp;
    ptzPelcodFuns->ptzPanTiltLeftDown = ptz_pelcod_panTilt_leftDown;
    ptzPelcodFuns->ptzZoomTele = ptz_pelcod_zoomTele;
    ptzPelcodFuns->ptzZoomWide = ptz_pelcod_zoomWide;
    ptzPelcodFuns->ptzFocusFar = ptz_pelcod_focusFar;
    ptzPelcodFuns->ptzFocusNear = ptz_pelcod_focusNear;
    ptzPelcodFuns->ptzIrisOpen = ptz_pelcod_irisOpen;
    ptzPelcodFuns->ptzIrisClose = ptz_pelcod_irisClose;
    ptzPelcodFuns->ptzAutoScan = ptz_pelcod_autoScan;
    ptzPelcodFuns->ptzSetPreset = ptz_pelcod_setPreset;
    ptzPelcodFuns->ptzClearPreset = ptz_pelcod_clearPreset;
    ptzPelcodFuns->ptzGoToPreset = ptz_pelcod_goToPreset;

    gadi_ptz_register_protocol(ptzControlHandle, protocolName, protocol, ptzPelcodCmd,
                            ptzPelcodFuns);

    return GADI_OK;
}

void gadi_ptz_pelcod_exit(GADI_PTZ_ControlHandleT  *ptzControlHandle)
{
    gadi_ptz_unregister_protocol(ptzControlHandle, protocol);
    gadi_sys_free(ptzPelcodCmd);
    ptzPelcodCmd = NULL;
    gadi_sys_free(ptzPelcodFuns);
    ptzPelcodFuns = NULL;

    return;
}

//*****************************************************************************
//*****************************************************************************
//** Local Functions
//*****************************************************************************
//*****************************************************************************
static GADI_U8 get_cmd1(GADI_U16 pelcodCmd)
{
    GADI_U8 cmd1;
    cmd1 = (GADI_U8)((pelcodCmd >> 8) & 0xFF);

    return cmd1;
}

static GADI_U8 get_cmd2(GADI_U16 pelcodCmd)
{
    GADI_U8 cmd2;
    cmd2 = (GADI_U8)(pelcodCmd & 0xFF);

    return cmd2;
}


static void ptz_pelcod_set_message(GADI_U8 cmd1, GADI_U8 cmd2,
                                GADI_U8 data1, GADI_U8 data2) {
    pelcodMsg.cmd1 = cmd1;
    pelcodMsg.cmd2 = cmd2;
    pelcodMsg.data1 = data1;
    pelcodMsg.data2 = data2;

    pelcodMsg.checkSum = pelcodMsg.addr +
                        pelcodMsg.cmd1 +
                        pelcodMsg.cmd2 +
                        pelcodMsg.data1 +
                        pelcodMsg.data2;

    return;
}

static void ptz_pelcod_set_address(GADI_U8 address)
{
    pelcodMsg.addr = address;

    return;
}

static void ptz_pelcod_stop()
{
    ptz_pelcod_set_message(get_cmd1(GADI_PTZ_PELCOD_STOP),
                            get_cmd2(GADI_PTZ_PELCOD_STOP), 0x00, 0x01);

    return;
}

static void ptz_pelcod_autoScan()
{
    ptz_pelcod_set_message(get_cmd1(GADI_PTZ_PELCOD_AUTO_SCAN),
                            get_cmd2(GADI_PTZ_PELCOD_AUTO_SCAN), 0x00, 0x00);

    return;
}

static void ptz_pelcod_panRight(GADI_U8 panSpeed)
{
    if(panSpeed > GADI_PTZ_PAN_SPEED_MAX) {
        panSpeed = GADI_PTZ_PAN_SPEED_MAX;
    }
    ptz_pelcod_set_message(get_cmd1(GADI_PTZ_PELCOD_PAN_RIGHT),
                            get_cmd2(GADI_PTZ_PELCOD_PAN_RIGHT), panSpeed, 0x00);

    return;
}

static void ptz_pelcod_panLeft(GADI_U8 panSpeed)
{
    if(panSpeed > GADI_PTZ_PAN_SPEED_MAX) {
        panSpeed = GADI_PTZ_PAN_SPEED_MAX;
    }
    ptz_pelcod_set_message(get_cmd1(GADI_PTZ_PELCOD_PAN_LEFT),
                            get_cmd2(GADI_PTZ_PELCOD_PAN_LEFT), panSpeed, 0x00);

    return;
}

static void ptz_pelcod_tiltUp(GADI_U8 tiltSpeed)
{
    if(tiltSpeed > GADI_PTZ_TILT_SPEED_MAX) {
        tiltSpeed = GADI_PTZ_TILT_SPEED_MAX;
    }
    ptz_pelcod_set_message(get_cmd1(GADI_PTZ_PELCOD_TILT_UP),
                            get_cmd2(GADI_PTZ_PELCOD_TILT_UP), 0x00, tiltSpeed);

    return;
}

static void ptz_pelcod_tiltDown(GADI_U8 tiltSpeed)
{
    if(tiltSpeed > GADI_PTZ_TILT_SPEED_MAX) {
        tiltSpeed = GADI_PTZ_TILT_SPEED_MAX;
    }
    ptz_pelcod_set_message(get_cmd1(GADI_PTZ_PELCOD_TILT_DOWN),
                            get_cmd2(GADI_PTZ_PELCOD_TILT_DOWN), 0x00, tiltSpeed);

    return;
}

static void ptz_pelcod_panTilt_rightUp(GADI_U8 panSpeed,
                                    GADI_U8 tiltSpeed) {
    if(panSpeed > GADI_PTZ_PAN_SPEED_MAX) {
        panSpeed = GADI_PTZ_PAN_SPEED_MAX;
    }
    if(tiltSpeed > GADI_PTZ_TILT_SPEED_MAX) {
        tiltSpeed = GADI_PTZ_TILT_SPEED_MAX;
    }
    ptz_pelcod_set_message(get_cmd1(GADI_PTZ_PELCOD_PANTILT_RIGHTUP),
                            get_cmd2(GADI_PTZ_PELCOD_PANTILT_RIGHTUP),
                            panSpeed, tiltSpeed);

    return;
}

static void ptz_pelcod_panTilt_rightDown(GADI_U8 panSpeed, GADI_U8 tiltSpeed)
{
    if(panSpeed > GADI_PTZ_PAN_SPEED_MAX) {
        panSpeed = GADI_PTZ_PAN_SPEED_MAX;
    }
    if(tiltSpeed > GADI_PTZ_TILT_SPEED_MAX) {
        tiltSpeed = GADI_PTZ_TILT_SPEED_MAX;
    }
    ptz_pelcod_set_message(get_cmd1(GADI_PTZ_PELCOD_PANTILT_RIGHTDOWN),
                            get_cmd2(GADI_PTZ_PELCOD_PANTILT_RIGHTDOWN), panSpeed,
                            tiltSpeed);

    return;
}

static void ptz_pelcod_panTilt_leftUp(GADI_U8 panSpeed, GADI_U8 tiltSpeed)
{
    if(panSpeed > GADI_PTZ_PAN_SPEED_MAX) {
        panSpeed = GADI_PTZ_PAN_SPEED_MAX;
    }
    if(tiltSpeed > GADI_PTZ_TILT_SPEED_MAX) {
        tiltSpeed = GADI_PTZ_TILT_SPEED_MAX;
    }
    ptz_pelcod_set_message(get_cmd1(GADI_PTZ_PELCOD_PANTILT_LEFTUP),
                            get_cmd2(GADI_PTZ_PELCOD_PANTILT_LEFTUP), panSpeed,
                            tiltSpeed);

    return;
}

static void ptz_pelcod_panTilt_leftDown(GADI_U8 panSpeed, GADI_U8 tiltSpeed)
{
    if(panSpeed > GADI_PTZ_PAN_SPEED_MAX) {
        panSpeed = GADI_PTZ_PAN_SPEED_MAX;
    }
    if(tiltSpeed > GADI_PTZ_TILT_SPEED_MAX) {
        tiltSpeed = GADI_PTZ_TILT_SPEED_MAX;
    }
    ptz_pelcod_set_message(get_cmd1(GADI_PTZ_PELCOD_PANTILT_LEFTDOWN),
                            get_cmd2(GADI_PTZ_PELCOD_PANTILT_LEFTDOWN), panSpeed,
                            tiltSpeed);

    return;
}

static void ptz_pelcod_zoomTele()
{
    ptz_pelcod_set_message(get_cmd1(GADI_PTZ_PELCOD_ZOOM_TELE),
                            get_cmd2(GADI_PTZ_PELCOD_ZOOM_TELE), 0x00, 0x00);

    return;
}

static void ptz_pelcod_zoomWide()
{
    ptz_pelcod_set_message(get_cmd1(GADI_PTZ_PELCOD_ZOOM_WIDE),
                            get_cmd2(GADI_PTZ_PELCOD_ZOOM_WIDE), 0x00, 0x00);

    return;
}

static void ptz_pelcod_focusFar()
{
    ptz_pelcod_set_message(get_cmd1(GADI_PTZ_PELCOD_FOCUS_FAR),
                            get_cmd2(GADI_PTZ_PELCOD_FOCUS_FAR), 0x00, 0x00);

    return;
}

static void ptz_pelcod_focusNear()
{
    ptz_pelcod_set_message(get_cmd1(GADI_PTZ_PELCOD_FOCUS_NEAR),
                            get_cmd2(GADI_PTZ_PELCOD_FOCUS_NEAR), 0x00, 0x00);

    return;
}

static void ptz_pelcod_irisOpen()
{
    ptz_pelcod_set_message(get_cmd1(GADI_PTZ_PELCOD_IRIS_OPEN),
                            get_cmd2(GADI_PTZ_PELCOD_IRIS_OPEN), 0x00, 0x00);

    return;
}

static void ptz_pelcod_irisClose()
{
    ptz_pelcod_set_message(get_cmd1(GADI_PTZ_PELCOD_IRIS_CLOSE),
                            get_cmd2(GADI_PTZ_PELCOD_IRIS_CLOSE), 0x00, 0x00);

    return;
}

static void ptz_pelcod_setPreset(GADI_U8 presetNum)
{
    ptz_pelcod_set_message(get_cmd1(GADI_PTZ_PELCOD_SET_PRESET),
                            get_cmd2(GADI_PTZ_PELCOD_SET_PRESET), 0x00, presetNum);

    return;
}

static void ptz_pelcod_clearPreset(GADI_U8 presetNum)
{
    ptz_pelcod_set_message(get_cmd1(GADI_PTZ_PELCOD_CLEAR_PRESET),
                            get_cmd2(GADI_PTZ_PELCOD_CLEAR_PRESET), 0x00, presetNum);

    return;
}

static void ptz_pelcod_goToPreset(GADI_U8 presetNum)
{
    ptz_pelcod_set_message(get_cmd1(GADI_PTZ_PELCOD_GOTO_PRESET),
                            get_cmd2(GADI_PTZ_PELCOD_GOTO_PRESET), 0x00, presetNum);

    return;
}

