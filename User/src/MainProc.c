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
#include "OLED.h"
#include "stdarg.h"
#include "stdio.h"
#include "Joystick.h"
#include "Gyro.h"

bool enabled;
extern int32_t preRingEndDistance;
extern bool preRingEnd;

static void NVICInit(void);
static void BuzzleInit(void);
static void TimerInit(void);
static void OLEDInit(void);
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
    
    JoystickInit();
    
    MotorInit();
    
    EncoderInit();
    
    SteerActuatorInit();
    
    GyroInit();
    
    DataCommInit();
    
    BuzzleInit();
    
    NVICInit();
    
    ImgProcInit();
    
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
    NVIC_SetPriority(JYSK_IRQ, NVIC_EncodePriority(NVIC_PriorityGroup_3, 3, 1));
}

void GetReady() {
    if(double_car) {
        if(leader_car) {
            while(!enabled) {
                
            }
            DelayMs(2000);
            SendMessage(START);
        } else {
            while(!enabled) {
                OLEDPrintf(5, 2, "D: %.3f", distanceBetweenTheTwoCars);
                DelayMs(100);
            }
            DelayMs(100);
        }
    } else {
        while(!enabled) {
            
        }
        DelayMs(2000);
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

void DistanceControl() {
    int16_t dist = (leftSpeed + rightSpeed) / 2;
    if(!startLineEnabled) {
        if(wholeDistance < 2000) {
            wholeDistance += dist;
        } else {
            startLineEnabled = true;
        }
    }
    if(inRing || ringEndDelay) {
        ringDistance += dist;
    }
    if(inCrossRoad) {
        crossRoadDistance += dist;
        if(crossRoadDistance > crossRoadDistanceMax) {
            inCrossRoad = false;
            crossRoadDistance = 0;
            afterCrossRoad = crossRoadOvertaking && crossRoadOvertakingCnt < 1;
        }
    }
    if(afterCrossRoad) {
        afterCrossRoadDistance += dist;
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
    if(holding) {
        holdingDistance += dist;
        if(holdingDistance > 4000) {
            holding = false;
            holdingDistance = 0;
        }
    }
    if(overtaking) {
        overtakingDistance += dist;
        if(overtakingDistance > overtakingDistanceMax) {
            overtakingDistance = 0;
            overtaking = false;
        }
    }
    if(sendOvertakingFinishedMsgLater) {
        sendOvertakingFinishedMsgLaterDistance += dist;
        if(sendOvertakingFinishedMsgLaterDistance > sendOvertakingFinishedMsgLaterDistanceMax) {
            sendOvertakingFinishedMsgLater = false;
            sendOvertakingFinishedMsgLaterDistance = 0;
            SendMessage(OVERTAKINGFINISHED);
            leader_car = !leader_car;
        }
    }
    if(onRamp) {
        rampDistance += dist;
    }
    if(inStraightLine) {
        straightLineDistance += dist;
    }
    if(preRingEnd){
        preRingEndDistance += dist;
        if(preRingEndDistance > 8000) {
            preRingEnd = false;
            preRingEndDistance = 0;
        }
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
    static int16_t ovtcnt = 0;
    
    if(encoder_on) {
        EncoderGet(&leftSpeed, &rightSpeed);
    } else {
        EncoderClear();
        leftSpeed = rightSpeed = 0;
    }
    
    BuzzleControl(final || inRing || ringEndDelay);
    
    DistanceControl();
    
//    if(beingOvertaken) {
//        if(++ovtcnt > 800) {
//            beingOvertaken = false;
//            ovtcnt = 0;
//            along = AsUsual;
//        }
//    }
    
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
    
    speed_control_acc = 5;
    speed_control_dec = 10;
    
    leftPid.targetValue = speed_control_speed;
    rightPid.targetValue = speed_control_speed;
    
    #if CAR_NO == 1
    
    leftPid.kp = 130;
    leftPid.ki = 15;
    leftPid.kd = 25;
    rightPid.kp = 130;
    rightPid.ki = 15;
    rightPid.kd = 25;
    
    steer_actuator_middle = 753;

    direction_control_kd = 0.2;
    direction_control_kpj = 0.04;
    direction_control_kpc = 0.000125; //0.0001
    
    differential_ratio = 0.036;  //0.034
    
    #elif CAR_NO == 2
    
    leftPid.kp = 135;
    leftPid.ki = 12;
    leftPid.kd = 25;
    rightPid.kp = 135;
    rightPid.ki = 12;
    rightPid.kd = 25;
    
    steer_actuator_middle = 750;
    
    direction_control_kd = 0.2;
    direction_control_kpj = 0.04;
    direction_control_kpc = 0.000125; //0.0001
    
    differential_ratio = 0.042;  //0.033
    
    #else
    
    #error "not implemented"
    
    #endif
    
    reduction_ratio = 2.55;
    avg_distance_between_the_two_cars = 130;
    diff_distance_max = 7;
    crossRoadDistanceMax = 2000;
    startLinePresight = 30;
    startLineWidth = 124;
    sendOvertakingFinishedMsgLaterDistanceMax = 9000;
    overtakingDistanceMax = 5000;
    speedAroundBarrier = 60;
    speedInRing = 95;
    out = true;
    crossRoadActionEnabled = true;
    
    ringOvertakingCntMax = 6;
    rampOvertakingCntMax = 1;
    straightLineOvertakingCntMax = 6;
    barrierOvertakingCntMax = 1;
}
