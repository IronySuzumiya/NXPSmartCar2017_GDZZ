#include "Joystick.h"
#include "gpio.h"
#include "SpeedControl.h"
#include "MainProc.h"
#include "DoubleCar.h"
#include "ImgProc.h"
#include "pit.h"
#include "ImgUtility.h"
#include "DirectionControl.h"
#include "ModeSwitch.h"

static int16_t joystickConfirmingCnt;

static struct _temp_handle {
    int16_t index;
    union {
        int16_t i16;
        int32_t i32;
        float f;
        bool b;
    } u;
} temp = {
    0, 0
};

static struct _group {
    enum { INT16, INT32, FLOAT, BOOL, ORDER } type;
    char *name;
    void *ref;
    float div;
} group[] = {
    { FLOAT, "reduction", &reduction_ratio, 0.02 },
    { FLOAT, "speeddiff", &differential_ratio, 0.0002 },
    { INT32, "avgdistance", &avg_distance_between_the_two_cars, 1 },
    { INT32, "diffdistmax", &diff_distance_max, 1 },
    { INT16, "speed", &speed_control_speed, 1 },
    { INT16, "presight", &pre_sight, 1 },
    { INT16, "ringspeed", &speedInRing, 1 },
    { BOOL, "out", &out, 1 },
    { INT16, "ringcnt", &ringOvertakingCntMax, 1 },
    { INT16, "rampcnt", &rampOvertakingCntMax, 1 },
    { INT16, "strlncnt", &straightLineOvertakingCntMax, 1 },
    { INT16, "barrcnt", &barrierOvertakingCntMax, 1 },
    { BOOL, "barrdouble", &barrierDoubleOvertakingEnabled, 1 },
    { ORDER, "ringorder", &ringOrder, 1 },
    { BOOL, "transimg", &trans_img, 1 },
    { BOOL, "transstat", &trans_stat, 1 },
    { BOOL, "dirlock", &dirlocked, 1 },
    { INT16, "dshdstlead", &finalDashDistanceLeader, 100 },
    { INT16, "dshdstfolw", &finalDashDistanceFollower, 100 },
    { INT16, "finaldist", &finalMinDistance, 1 },
    { INT16, "dshspdlead", &finalDashSpeedLeader, 1 },
    { INT16, "dshspdfolw", &finalDashSpeedFollower, 1 },
};

static struct _param_handle {
    int16_t index;
    struct _group *group;
} param = {
    0,
    group
};

static void JoystickConfirmingInt(void);
static void ParamShow(void);
static void ParamFetch(void);
static void ParamInc(void);
static void ParamDec(void);
static void ParamUpdate(void);

void JoystickInit() {
    GPIO_QuickInit(JOYSTICK_PORT, JOYSTICK_NORTH, kGPIO_Mode_IPU);
    GPIO_QuickInit(JOYSTICK_PORT, JOYSTICK_SOUTH, kGPIO_Mode_IPU);
    GPIO_QuickInit(JOYSTICK_PORT, JOYSTICK_WEST, kGPIO_Mode_IPU);
    GPIO_QuickInit(JOYSTICK_PORT, JOYSTICK_EAST, kGPIO_Mode_IPU);
    GPIO_QuickInit(JOYSTICK_PORT, JOYSTICK_MIDDLE, kGPIO_Mode_IPU);
    
    GPIO_CallbackInstall(JOYSTICK_PORT, ImgProcVSYN); // it sucks
    
    GPIO_ITDMAConfig(JOYSTICK_PORT, JOYSTICK_NORTH, kGPIO_IT_RisingEdge, ENABLE);
    GPIO_ITDMAConfig(JOYSTICK_PORT, JOYSTICK_SOUTH, kGPIO_IT_RisingEdge, ENABLE);
    GPIO_ITDMAConfig(JOYSTICK_PORT, JOYSTICK_WEST, kGPIO_IT_RisingEdge, ENABLE);
    GPIO_ITDMAConfig(JOYSTICK_PORT, JOYSTICK_EAST, kGPIO_IT_RisingEdge, ENABLE);
    GPIO_ITDMAConfig(JOYSTICK_PORT, JOYSTICK_MIDDLE, kGPIO_IT_RisingFallingEdge, ENABLE);
    
    PIT_QuickInit(JOYSTICK_CONFIRMING_TIMER_CHL, JOYSTICK_CONFIRMING_TIME);
    PIT_CallbackInstall(JOYSTICK_CONFIRMING_TIMER_CHL, JoystickConfirmingInt);
    PIT_ITDMAConfig(JOYSTICK_CONFIRMING_TIMER_CHL, kPIT_IT_TOF, DISABLE);
    
    ParamFetch();
    ParamShow();
}

void JoystickInt(uint32_t pinxArray) {
    if(pinxArray & (1 << JOYSTICK_NORTH)) {
        ParamInc();
        ParamShow();
    } else if(pinxArray & (1 << JOYSTICK_SOUTH)) {
        ParamDec();
        ParamShow();
    } else if(pinxArray & (1 << JOYSTICK_WEST)) {
        if(--param.index < 0) {
            param.index = MODIFIABLE_PARAM_NUM - 1;
        }
        ParamFetch();
        ParamShow();
    } else if(pinxArray & (1 << JOYSTICK_EAST)) {
        param.index = (param.index + 1) % MODIFIABLE_PARAM_NUM;
        ParamFetch();
        ParamShow();
    } else if(pinxArray & (1 << JOYSTICK_MIDDLE)) {
        if(JOYSTICK_MIDDLE_READ) {
            if(joystickConfirmingCnt >= 20) {
                OLEDPrintf(70, 3, "GoGoGo");
                enabled = true;
            } else {
                ParamUpdate();
                ParamShow();
                OLEDPrintf(70, 3, "OK!");
            }
            PIT_ITDMAConfig(JOYSTICK_CONFIRMING_TIMER_CHL, kPIT_IT_TOF, DISABLE);
            joystickConfirmingCnt = 0;
        } else {
            PIT_ResetCounter(JOYSTICK_CONFIRMING_TIMER_CHL);
            PIT_ITDMAConfig(JOYSTICK_CONFIRMING_TIMER_CHL, kPIT_IT_TOF, ENABLE);
        }
    }
}

void JoystickConfirmingInt() {
    ++joystickConfirmingCnt;
}

void ParamShow() {
    OLEDClrRow(3);
    switch(param.group[temp.index].type) {
        case INT16:
            OLEDPrintf(5, 3, "%s: %d", param.group[temp.index].name, temp.u.i16);
            break;
        case INT32:
            OLEDPrintf(5, 3, "%s: %d", param.group[temp.index].name, temp.u.i32);
            break;
        case FLOAT:
            OLEDPrintf(5, 3, "%s: %.5f", param.group[temp.index].name, temp.u.f);
            break;
        case BOOL:
            OLEDPrintf(5, 3, "%s: %s", param.group[temp.index].name, temp.u.b ? "true" : "false");
            break;
        case ORDER:
            OLEDPrintf(5, 3, "%s: %x", param.group[temp.index].name, temp.u.i16);
    }
}

void ParamFetch() {
    temp.index = param.index;
    switch(param.group[temp.index].type) {
        case INT16:
            temp.u.i16 = *(int16_t *)(param.group[temp.index].ref);
            break;
        case INT32:
            temp.u.i32 = *(int32_t *)(param.group[temp.index].ref);
            break;
        case FLOAT:
            temp.u.f = *(float *)(param.group[temp.index].ref);
            break;
        case BOOL:
            temp.u.b = *(bool *)(param.group[temp.index].ref);
            break;
        case ORDER:
            temp.u.i16 = *(int16_t *)(param.group[temp.index].ref);
            break;
    }
}

void ParamInc() {
    switch(param.group[temp.index].type) {
        case INT16:
            temp.u.i16 += param.group[temp.index].div;
            break;
        case INT32:
            temp.u.i32 += param.group[temp.index].div;
            break;
        case FLOAT:
            temp.u.f += param.group[temp.index].div;
            break;
        case BOOL:
            temp.u.b = !temp.u.b;
            break;
        case ORDER:
            temp.u.i16 += param.group[temp.index].div;
            break;
    }
}

void ParamDec() {
    switch(param.group[temp.index].type) {
        case INT16:
            temp.u.i16 -= param.group[temp.index].div;
            break;
        case INT32:
            temp.u.i32 -= param.group[temp.index].div;
            break;
        case FLOAT:
            temp.u.f -= param.group[temp.index].div;
            break;
        case BOOL:
            temp.u.b = !temp.u.b;
            break;
        case ORDER:
            temp.u.i16 -= param.group[temp.index].div;
            break;
    }
}

void ParamUpdate() {
    switch(param.group[temp.index].type) {
        case INT16:
            *(int16_t *)(param.group[temp.index].ref) = temp.u.i16;
            break;
        case INT32:
            *(int32_t *)(param.group[temp.index].ref) = temp.u.i32;
            break;
        case FLOAT:
            *(float *)(param.group[temp.index].ref) = temp.u.f;
            break;
        case BOOL:
            *(bool *)(param.group[temp.index].ref) = temp.u.b;
            break;
        case ORDER:
            *(int16_t *)(param.group[temp.index].ref) = temp.u.i16;
            break;
    }
}
