#include "Joystick.h"
#include "gpio.h"
#include "SpeedControl.h"
#include "MainProc.h"
#include "DoubleCar.h"

static struct _temp_handle {
    int16_t index;
    union {
        int16_t i16;
        int32_t i32;
        float f;
    } u;
} temp = {
    0, 0
};

static struct _param_handle {
    int16_t index;
    enum { INT16, INT32, FLOAT } type[MODIFIABLE_PARAM_NUM];
    char *name[MODIFIABLE_PARAM_NUM];
    void *(ref[MODIFIABLE_PARAM_NUM]);
    float div[MODIFIABLE_PARAM_NUM];
} param = {
    0,
    { FLOAT, FLOAT, INT32, INT32, INT32 },
    { "redu", "diff", "wait", "ovtk", "arov" },
    { &reduction_ratio, &differential_ratio, &waitForOvertakingTimeMax,
        &overtakingTime, &aroundOvertakingTimeMax },
    { 0.02, 0.0002, 5, 2, 2 }
};

static void JoystickInt(uint32_t pinxArray);
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
    GPIO_CallbackInstall(JOYSTICK_PORT, JoystickInt);
    GPIO_ITDMAConfig(JOYSTICK_PORT, JOYSTICK_NORTH, kGPIO_IT_FallingEdge, ENABLE);
    GPIO_ITDMAConfig(JOYSTICK_PORT, JOYSTICK_SOUTH, kGPIO_IT_FallingEdge, ENABLE);
    GPIO_ITDMAConfig(JOYSTICK_PORT, JOYSTICK_WEST, kGPIO_IT_FallingEdge, ENABLE);
    GPIO_ITDMAConfig(JOYSTICK_PORT, JOYSTICK_EAST, kGPIO_IT_FallingEdge, ENABLE);
    GPIO_ITDMAConfig(JOYSTICK_PORT, JOYSTICK_MIDDLE, kGPIO_IT_FallingEdge, ENABLE);
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
        ParamUpdate();
        ParamShow();
        DelayMs(100);
        OLEDPrintf(18, 3, "OK!");
    }
}

void ParamShow() {
    OLEDClrRow(3);
    DelayMs(100);
    switch(param.type[temp.index]) {
        case INT16:
            OLEDPrintf(5, 3, "%s: %d", param.name[temp.index], temp.u.i16);
            break;
        case INT32:
            OLEDPrintf(5, 3, "%s: %d", param.name[temp.index], temp.u.i32);
            break;
        case FLOAT:
            OLEDPrintf(5, 3, "%s: %.4f", param.name[temp.index], temp.u.f);
            break;
    }
}

void ParamFetch() {
    temp.index = param.index;
    switch(param.type[param.index]) {
        case INT16:
            temp.u.i16 = *(int16_t *)(param.ref[param.index]);
            break;
        case INT32:
            temp.u.i32 = *(int32_t *)(param.ref[param.index]);
            break;
        case FLOAT:
            temp.u.f = *(float *)(param.ref[param.index]);
            break;
    }
}

void ParamInc() {
    switch(param.type[temp.index]) {
        case INT16:
            temp.u.i16 += param.div[temp.index];
            break;
        case INT32:
            temp.u.i32 += param.div[temp.index];
            break;
        case FLOAT:
            temp.u.f += param.div[temp.index];
            break;
    }
}

void ParamDec() {
    switch(param.type[temp.index]) {
        case INT16:
            temp.u.i16 -= param.div[temp.index];
            break;
        case INT32:
            temp.u.i32 -= param.div[temp.index];
            break;
        case FLOAT:
            temp.u.f -= param.div[temp.index];
            break;
    }
}

void ParamUpdate() {
    switch(param.type[temp.index]) {
        case INT16:
            *(int16_t *)(param.ref[temp.index]) = temp.u.i16;
            break;
        case INT32:
            *(int32_t *)(param.ref[temp.index]) = temp.u.i32;
            break;
        case FLOAT:
            *(float *)(param.ref[temp.index]) = temp.u.f;
            break;
    }
}
