#include "Joystick.h"
#include "gpio.h"
#include "SpeedControl.h"
#include "MainProc.h"
#include "DoubleCar.h"
#include "ImgProc.h"
#include "pit.h"
#include "ImgUtility.h"
#include "DirectionControl.h"

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

static struct _param_handle {
    int16_t index;
    enum { INT16, INT32, FLOAT, BOOL } type[MODIFIABLE_PARAM_NUM];
    char *name[MODIFIABLE_PARAM_NUM];
    void *(ref[MODIFIABLE_PARAM_NUM]);
    float div[MODIFIABLE_PARAM_NUM];
} param = {
    0,
    { FLOAT, FLOAT, INT32,
      INT32, INT16, INT16,
      INT16, INT16, FLOAT,
      FLOAT, FLOAT, INT16,
      INT16, BOOL,  BOOL,
      INT32, INT32, BOOL },
    { "reduction",   "speeddiff",  "avgdistance",
      "diffdistmax", "speed",      "presight",
      "sttlnprst",   "sttlnwidth", "dirkpj",
      "dirkpc",      "dirkd",      "barrspeed",
      "ringspeed",   "out",        "crsraction",
      "sttlnedist",  "barovtkdst", "finalsync" },
    { &reduction_ratio,         &differential_ratio,           &avg_distance_between_the_two_cars,
      &diff_distance_max,       &speed_control_speed,          &pre_sight,
      &startLinePresight,       &startLineWidth,               &direction_control_kpj,
      &direction_control_kpc,   &direction_control_kd,         &speedAroundBarrier,
      &speedInRing,             &out,                          &crossRoadActionEnabled,
      &startLineEnableDistance, &barrierOvertakingDistanceMax, &final_sync },
    { 0.02,    0.0002, 1,
      1,       1,      1,
      1,       2,      0.0001,
      0.00001, 0.01,   2,
      1,       1,      1,
      100,     100,    1 }
};

static void JoystickConfirmingInt(void);
#if CAR_NO == 1
static void JoystickInt(uint32_t pinxArray);
#elif CAR_NO == 2
static void JoystickInt1(uint32_t pinxArray);
static void JoystickInt2(uint32_t pinxArray);
#endif
static void ParamShow(void);
static void ParamFetch(void);
static void ParamInc(void);
static void ParamDec(void);
static void ParamUpdate(void);

void JoystickInit() {
    #if CAR_NO == 1
    GPIO_QuickInit(JOYSTICK_PORT, JOYSTICK_SOUTH, kGPIO_Mode_IPU);
    GPIO_QuickInit(JOYSTICK_PORT, JOYSTICK_WEST, kGPIO_Mode_IPU);
    GPIO_QuickInit(JOYSTICK_PORT, JOYSTICK_MIDDLE, kGPIO_Mode_IPU);
    GPIO_QuickInit(JOYSTICK_PORT, JOYSTICK_NORTH, kGPIO_Mode_IPU);
    
    GPIO_CallbackInstall(JOYSTICK_PORT, JoystickInt);
    
    GPIO_ITDMAConfig(JOYSTICK_PORT, JOYSTICK_SOUTH, kGPIO_IT_RisingEdge, ENABLE);
    GPIO_ITDMAConfig(JOYSTICK_PORT, JOYSTICK_WEST, kGPIO_IT_RisingFallingEdge, ENABLE);
    GPIO_ITDMAConfig(JOYSTICK_PORT, JOYSTICK_MIDDLE, kGPIO_IT_RisingFallingEdge, ENABLE);
    GPIO_ITDMAConfig(JOYSTICK_PORT, JOYSTICK_NORTH, kGPIO_IT_RisingEdge, ENABLE);
    #elif CAR_NO == 2
    GPIO_QuickInit(JOYSTICK_OLD_PORT, JOYSTICK_SOUTH, kGPIO_Mode_IPU);
    GPIO_QuickInit(JOYSTICK_OLD_PORT, JOYSTICK_WEST, kGPIO_Mode_IPU);
    GPIO_QuickInit(JOYSTICK_OLD_PORT, JOYSTICK_MIDDLE, kGPIO_Mode_IPU);
    
    GPIO_QuickInit(JOYSTICK_NEW_PORT, JOYSTICK_NORTH, kGPIO_Mode_IPU);
    GPIO_QuickInit(JOYSTICK_NEW_PORT, JOYSTICK_EAST, kGPIO_Mode_IPU);
    
    GPIO_CallbackInstall(JOYSTICK_OLD_PORT, JoystickInt1);
    GPIO_CallbackInstall(JOYSTICK_NEW_PORT, JoystickInt2);
    
    GPIO_ITDMAConfig(JOYSTICK_OLD_PORT, JOYSTICK_SOUTH, kGPIO_IT_RisingEdge, ENABLE);
    GPIO_ITDMAConfig(JOYSTICK_OLD_PORT, JOYSTICK_WEST, kGPIO_IT_RisingEdge, ENABLE);
    GPIO_ITDMAConfig(JOYSTICK_OLD_PORT, JOYSTICK_MIDDLE, kGPIO_IT_RisingFallingEdge, ENABLE);
    
    GPIO_ITDMAConfig(JOYSTICK_NEW_PORT, JOYSTICK_NORTH, kGPIO_IT_RisingEdge, ENABLE);
    GPIO_ITDMAConfig(JOYSTICK_NEW_PORT, JOYSTICK_EAST, kGPIO_IT_RisingEdge, ENABLE);
    #endif
    
    PIT_QuickInit(JOYSTICK_CONFIRMING_TIMER_CHL, JOYSTICK_CONFIRMING_TIME);
    PIT_CallbackInstall(JOYSTICK_CONFIRMING_TIMER_CHL, JoystickConfirmingInt);
    PIT_ITDMAConfig(JOYSTICK_CONFIRMING_TIMER_CHL, kPIT_IT_TOF, DISABLE);
    
    ParamFetch();
    ParamShow();
}

#if CAR_NO == 1
void JoystickInt(uint32_t pinxArray) {
    if(pinxArray & (1 << JOYSTICK_SOUTH)) {
        ParamDec();
        ParamShow();
    } else if(pinxArray & (1 << JOYSTICK_WEST)) {
        if(JOYSTICK_WEST_READ) {
            if(joystickConfirmingCnt >= 15) {
                param.index = (param.index + 1) % MODIFIABLE_PARAM_NUM;
                ParamFetch();
                ParamShow();
            } else {
                if(--param.index < 0) {
                    param.index = MODIFIABLE_PARAM_NUM - 1;
                }
                ParamFetch();
                ParamShow();
            }
            PIT_ITDMAConfig(JOYSTICK_CONFIRMING_TIMER_CHL, kPIT_IT_TOF, DISABLE);
            joystickConfirmingCnt = 0;
        } else {
            PIT_ResetCounter(JOYSTICK_CONFIRMING_TIMER_CHL);
            PIT_ITDMAConfig(JOYSTICK_CONFIRMING_TIMER_CHL, kPIT_IT_TOF, ENABLE);
        }
    } else if(pinxArray & (1 << JOYSTICK_MIDDLE)) {
        if(JOYSTICK_MIDDLE_READ) {
            if(joystickConfirmingCnt >= 20) {
                OLEDPrintf(70, 3, "GoGoGo");
                enabled = true;
            } else {
                ParamUpdate();
                ParamShow();
                DelayMs(5);
                OLEDPrintf(70, 3, "OK!");
            }
            PIT_ITDMAConfig(JOYSTICK_CONFIRMING_TIMER_CHL, kPIT_IT_TOF, DISABLE);
            joystickConfirmingCnt = 0;
        } else {
            PIT_ResetCounter(JOYSTICK_CONFIRMING_TIMER_CHL);
            PIT_ITDMAConfig(JOYSTICK_CONFIRMING_TIMER_CHL, kPIT_IT_TOF, ENABLE);
        }
    } else if(pinxArray & (1 << JOYSTICK_NORTH)) {
        ParamInc();
        ParamShow();
    }
}
#elif CAR_NO == 2
void JoystickInt1(uint32_t pinxArray) {
    if(pinxArray & (1 << JOYSTICK_SOUTH)) {
        ParamDec();
        ParamShow();
    } else if(pinxArray & (1 << JOYSTICK_WEST)) {
        if(--param.index < 0) {
            param.index = MODIFIABLE_PARAM_NUM - 1;
        }
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
                DelayMs(5);
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

void JoystickInt2(uint32_t pinxArray) {
    if(pinxArray & (1 << JOYSTICK_NORTH)) {
        ParamInc();
        ParamShow();
    } else if(pinxArray & (1 << JOYSTICK_EAST)) {
        param.index = (param.index + 1) % MODIFIABLE_PARAM_NUM;
        ParamFetch();
        ParamShow();
    }
}
#endif

void JoystickConfirmingInt() {
    ++joystickConfirmingCnt;
}

void ParamShow() {
    OLEDClrRow(3);
    DelayMs(5);
    switch(param.type[temp.index]) {
        case INT16:
            OLEDPrintf(5, 3, "%s: %d", param.name[temp.index], temp.u.i16);
            break;
        case INT32:
            OLEDPrintf(5, 3, "%s: %d", param.name[temp.index], temp.u.i32);
            break;
        case FLOAT:
            OLEDPrintf(5, 3, "%s: %.5f", param.name[temp.index], temp.u.f);
            break;
        case BOOL:
            OLEDPrintf(5, 3, "%s: %s", param.name[temp.index], temp.u.b ? "true" : "false");
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
        case BOOL:
            temp.u.b = *(bool *)(param.ref[param.index]);
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
        case BOOL:
            temp.u.b = !temp.u.b;
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
        case BOOL:
            temp.u.b = !temp.u.b;
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
        case BOOL:
            *(bool *)(param.ref[temp.index]) = temp.u.b;
            break;
    }
}
