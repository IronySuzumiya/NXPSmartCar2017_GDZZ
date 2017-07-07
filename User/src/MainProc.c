#include "MainProc.h"
#include "ImgUtility.h"
#include "pit.h"
#include "gpio.h"
#include "Motor.h"
#include "Encoder.h"
#include "SteerActuator.h"
#include "DataComm.h"
#include "ImgProc.h"
#include "SpeedControl.h"
#include "DirectionControl.h"
#include "ModeSwitch.h"
#include "DoubleCar.h"
#include "GearSwitch.h"
#include "LQ12864.h"
#include "stdarg.h"
#include "stdio.h"
#include "Joystick.h"

bool enabled;
int16_t waitForOvertakingCnt;
int16_t overtakingCnt;

static void NVICInit(void);
static void BuzzleInit(void);
static void TimerInit(void);
static void OLEDInit(void);
static void OvertakingControl(void);
static void DistanceControl(void);
static void BuzzleControl(bool);
static void MainProc(void);
static void SwitchAndParamLoad(void);
static void GetReady(void);

void MainInit() {
    DelayInit();
    
    SwitchAndParamLoad();
    
    OLEDInit();
    
    ModeSelect();
    
    GearSelect();
    
    MotorInit();
    
    EncoderInit();
    
    SteerActuatorInit();
    
    DataCommInit();
    
    BuzzleInit();
    
    NVICInit();
    
    ImgProcInit();
    
    JoystickInit();
    
    if(double_car) {
        DoubleCarInit();
    }
    
    GetReady();
    
    TimerInit();
}

void NVICInit() {
    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_3);
    
    NVIC_SetPriority(VSYN_IRQ, NVIC_EncodePriority(NVIC_PriorityGroup_3, 0, 0));
    NVIC_SetPriority(HREF_IRQ, NVIC_EncodePriority(NVIC_PriorityGroup_3, 0, 1));
    
    NVIC_SetPriority(TIMR_IRQ, NVIC_EncodePriority(NVIC_PriorityGroup_3, 1, 0));
    
    NVIC_SetPriority(DCDT_IRQ, NVIC_EncodePriority(NVIC_PriorityGroup_3, 2, 0));
    NVIC_SetPriority(ULTR_IRQ, NVIC_EncodePriority(NVIC_PriorityGroup_3, 2, 1));
    
    NVIC_SetPriority(ULTO_IRQ, NVIC_EncodePriority(NVIC_PriorityGroup_3, 3, 0));
    NVIC_SetPriority(DCTO_IRQ, NVIC_EncodePriority(NVIC_PriorityGroup_3, 3, 1));
}

void GetReady() {
    if(double_car) {
        if(leader_car) {
            DelayMs(2000);
            SendMessage(START);
        } else {
            while(!enabled) {
                OLEDPrintf(5, 2, "distance: %.3f", distanceBetweenTheTwoCars);
                DelayMs(500);
            }
            DelayMs(100);
        }
    } else {
        GPIO_QuickInit(START_PORT, START_PIN, kGPIO_Mode_IPU);
        DelayMs(2000);
        SendMessage(START);
    }
}

void BuzzleInit() {
    GPIO_QuickInit(BUZZLE_PORT, BUZZLE_PIN, kGPIO_Mode_OPP);
}

void TimerInit() {
    PIT_QuickInit(PIT_CHL, PIT_PRD);
    PIT_CallbackInstall(PIT_CHL, MainProc);
    PIT_ITDMAConfig(PIT_CHL, kPIT_IT_TOF, ENABLE);
}

void OLEDInit() {
    OLED_Init();
    OLEDPrintf(0, 0, "Nadeko-AI");
}

void OLEDClrRow(uint8_t row) {
    // 21 spaces
    OLEDPrintf(0, row, "                     ");
}

void OLEDPrintf(uint8_t x, uint8_t y, char *str, ...) {
    static char buf[255];
    va_list ap;
    va_start(ap, str);
    
    vsprintf(buf, str, ap);
    OLED_P6x8Str(x, y, (uint8_t*)buf);
}

void OvertakingControl() {
    static bool aroundOvertakingFlag = false;
    static int aroundOvertakingCnt = 0;
    if(waitForOvertaking) {
        ++waitForOvertakingCnt;
        if(waitForOvertakingCnt > waitForOvertakingTimeMax) {
            overtaking = true;
            waitForOvertakingCnt = 0;
            overtakingCnt = overtakingTime;
        }
    } else {
        waitForOvertakingCnt = 0;
    }
    if(overtaking) {
        ++overtakingCnt;
        if(overtakingCnt > overtakingTime) {
            overtakingCnt = 0;
            overtaking = false;
            waitForOvertaking = false;
            aroundOvertakingFlag = true;
            leader_car = !leader_car;
        }
    }
    if(aroundOvertakingFlag) {
        ++aroundOvertakingCnt;
        if(aroundOvertakingCnt > aroundOvertakingTimeMax) {
            aroundOvertaking = false;
            aroundOvertakingFlag = false;
            aroundOvertakingCnt = 0;
        }
    }
}

void DistanceControl() {
    int16_t dist = (leftSpeed + rightSpeed) / 2 * 5;
    if(!startLineEnabled) {
        if(wholeDistance < 100000L) {
            wholeDistance += dist;
        } else {
            startLineEnabled = true;
        }
    }
    if(inRing || ringEndDelay || ringInterval) {
        ringDistance += dist;
    }
    if(inCrossRoad) {
        crossRoadDistance += dist;
    }
    if(aroundBarrier) {
        barrierDistance += dist;
    }
    if(!firstOvertakingFinished) {
        startDistance += dist;
    }
    if(final) {
        finalDistance += dist;
    }
    if(finalPursueingFinished) {
        dashDistance += dist;
    }
}

void BuzzleControl(bool flag) {
    static int cnt = 0;
    if(++cnt > 50) {
        cnt = 0;
        BUZZLE_OFF;
    }
    if(flag) {
        BUZZLE_ON;
    }
}

void MainProc() {
    if(encoder_on) {
        EncoderGet(&leftSpeed, &rightSpeed);
    } else {
        EncoderClear();
        leftSpeed = rightSpeed = 0;
    }
    
    BuzzleControl(aroundBarrier);
    
    OvertakingControl();
    
    DistanceControl();
    
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
    
    speed_control_speed = 90;
    speed_control_sum_err_max = 2000;
    
    speed_control_acc = 10;
    speed_control_dec = 20;
    
    leftPid.targetValue = speed_control_speed;
    rightPid.targetValue = speed_control_speed;
    
    #if CAR_NO == 1
    
    leftPid.kp = 130;
    leftPid.ki = 15;
    leftPid.kd = 25;
    rightPid.kp = 130;
    rightPid.ki = 15;
    rightPid.kd = 25;
    
    steer_actuator_right = 415;
    steer_actuator_middle = 452;
    steer_actuator_left = 555;
    pre_sight = pre_sight_default = 21;
    
    direction_control_kd = 0.2;
    direction_control_kpj = 0.02;
    direction_control_kpc = 0.0001;
    
    #elif CAR_NO == 2
    
    leftPid.kp = 135;
    leftPid.ki = 12;
    leftPid.kd = 25;
    rightPid.kp = 135;
    rightPid.ki = 12;
    rightPid.kd = 25;
    
    steer_actuator_right = 340;
    steer_actuator_middle = 410;
    steer_actuator_left = 480;
    pre_sight = pre_sight_default = 21;
    
    direction_control_kd = 0.2;
    direction_control_kpj = 0.025;
    direction_control_kpc = 0.0001;
    
    #else
    
    #error "not implemented"
    
    #endif
    
    reduction_ratio = 2.6;
    differential_ratio = 0.031;
    waitForOvertakingTimeMax = 700;
    overtakingTime = 200;
    aroundOvertakingTimeMax = 300;
    
    avg_distance_between_the_two_cars = 80;
    diff_distance_max = 15;
    
    dynamic_presight = true;
    presight_only_depends_on_pursueing = true;
}
