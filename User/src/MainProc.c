#include "MainProc.h"
#include "ImgUtility.h"
#include "pit.h"
#include "stdlib.h"
#include "gpio.h"

static void NVICInit(void);
static void BuzzleInit(void);
static void TimerInit(void);
static void MainProc(void);
static void SwitchAndParamLoad(void);

void MainInit() {
    DelayInit();
    
    SwitchAndParamLoad();
    
    ModeSelect();
    
    MotorInit();
    
    EncoderInit();
    
    SteerActuatorInit();
    
    DataCommInit();
    
    BuzzleInit();
    
    NVICInit();
    
    ImgProcInit();
    
    #ifdef DOUBLE_CAR
    DoubleCarRelativeInit();
    #endif
    
    TimerInit();
}

void NVICInit() {
    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_3);
    
    NVIC_SetPriority(VSYN_IRQ, NVIC_EncodePriority(NVIC_PriorityGroup_3, 0, 0));
    NVIC_SetPriority(HREF_IRQ, NVIC_EncodePriority(NVIC_PriorityGroup_3, 0, 1));
    
    NVIC_SetPriority(TIMR_IRQ, NVIC_EncodePriority(NVIC_PriorityGroup_3, 1, 0));
    
    NVIC_SetPriority(DCDT_IRQ, NVIC_EncodePriority(NVIC_PriorityGroup_3, 2, 0));
    NVIC_SetPriority(ULTR_IRQ, NVIC_EncodePriority(NVIC_PriorityGroup_3, 2, 1));
    NVIC_SetPriority(ULTO_IRQ, NVIC_EncodePriority(NVIC_PriorityGroup_3, 2, 2));
    NVIC_SetPriority(DCTO_IRQ, NVIC_EncodePriority(NVIC_PriorityGroup_3, 2, 3));
}

void BuzzleInit() {
    GPIO_QuickInit(BUZZLE_PORT, BUZZLE_PIN, kGPIO_Mode_OPP);
}

void TimerInit() {
    PIT_QuickInit(PIT_CHL, PIT_PRD);
    PIT_CallbackInstall(PIT_CHL, MainProc);
    PIT_ITDMAConfig(PIT_CHL, kPIT_IT_TOF, ENABLE);
}

void MainProc() {
    static int cnt = 0;
    static int waitForOvertakingCnt = 0;
    static int overtakingcnt = 0;
    static bool aroundOvertakingFlag = false;
    static int aroundOvertakingCnt = 0;
    if(cnt > 500) {
        cnt = 0;
        BUZZLE_OFF;
    }
    ++cnt;
    if(inRing) {
        BUZZLE_ON;
    }
    if(waitForOvertaking) {
        ++waitForOvertakingCnt;
        if(waitForOvertakingCnt > 250) {
            overtaking = true;
            overtakingcnt = 50;
        }
    } else {
        waitForOvertakingCnt = 0;
    }
    if(overtaking) {
        ++overtakingcnt;
        if(overtakingcnt > 50) {
            overtakingcnt = 0;
            overtaking = false;
            waitForOvertaking = false;
            aroundOvertakingFlag = true;
            front_car = !front_car;
        }
    }
    if(aroundOvertakingFlag) {
        ++aroundOvertakingCnt;
        if(aroundOvertakingCnt > 200) {
            aroundOvertaking = false;
            aroundOvertakingFlag = false;
            aroundOvertakingCnt = 0;
        }
    }
    EncoderGet(&leftSpeed, &rightSpeed);
    if(inRing || ringEndDelay) {
        ringDistance += (leftSpeed + rightSpeed) / 2 * 5;
    }
    if(inCrossRoad) {
        crossRoadDistance += (leftSpeed + rightSpeed) / 2 * 5;
    }
    if(aroundBarrier) {
        barrierDistance += (leftSpeed + rightSpeed) / 2 * 5;
    }
    if(speed_control_on) {
        SpeedControlProc(leftSpeed, rightSpeed);
    }
}

static void SwitchAndParamLoad() {
    motor_on = true;
    encoder_on = true;
    speed_control_on = true;
    direction_control_on = true;
    steer_actuator_on = true;
    img_trans_on = false;
    state_trans_on = false;
    mode_switch_on = false;
    
    speed_control_speed = 85;
    speed_control_sum_err_max = 2000;
    
    speed_control_acc_speed = 87;
    speed_control_dec_speed = 83;
    
    leftPid.targetValue = speed_control_speed;
    rightPid.targetValue = speed_control_speed;
    
    #ifdef NO1
    leftPid.kp = 130;
    leftPid.ki = 15;
    leftPid.kd = 25;
    rightPid.kp = 130;
    rightPid.ki = 15;
    rightPid.kd = 25;
    #else
    leftPid.kp = 135;
    leftPid.ki = 12;
    leftPid.kd = 25;
    rightPid.kp = 135;
    rightPid.ki = 12;
    rightPid.kd = 25;
    #endif
    
    #ifdef NO1
    steer_actuator_right = 415;
    steer_actuator_middle = 452;
    steer_actuator_left = 555;
    pre_sight = 21;
    #else
    steer_actuator_right = 340;
    steer_actuator_middle = 410;
    steer_actuator_left = 480;
    pre_sight = 20;
    #endif
    
    #ifdef NO1
    direction_control_kd = 0.2;
    direction_control_kpj = 0.02;
    direction_control_kpc = 0.0001;
    #else
    direction_control_kd = 0.2;
    direction_control_kpj = 0.025;
    direction_control_kpc = 0.0001;
    #endif
    
    #ifdef NO1
    front_car = true;
    #else
    front_car = false;
    #endif
}
