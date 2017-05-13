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
    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_2);
    NVIC_SetPriority(HREF_IRQ, NVIC_EncodePriority(NVIC_PriorityGroup_2, 0, 1));
    NVIC_SetPriority(VSYN_IRQ, NVIC_EncodePriority(NVIC_PriorityGroup_2, 0, 0));
    NVIC_SetPriority(TIMR_IRQ, NVIC_EncodePriority(NVIC_PriorityGroup_2, 1, 0));
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
    if(cnt > 100) {
        cnt = 0;
        BUZZLE_OFF;
    }
    ++cnt;
    EncoderGet(&leftSpeed, &rightSpeed);
    if(inRing || ringEndDelay) {
        ringDistance += (leftSpeed + rightSpeed) / 2 * 5;
    }
    if(aroundBarrier) {
        barrierDistance += (leftSpeed + rightSpeed) / 2 * 5;
    }
    if(speed_control_on) {
        SpeedControlProc(leftSpeed, rightSpeed);
    }
}

static void SwitchAndParamLoad(void) {
    motor_on = true;
    encoder_on = true;
    speed_control_on = true;
    direction_control_on = true;
    steer_actuator_on = true;
    img_trans_on = true;
    state_trans_on = false;
    mode_switch_on = false;
    
    speed_control_speed = 100;
    speed_control_sum_err_max = 2000;
    
    speed_control_acc_speed = 90;
    speed_control_dec_speed = 50;
    
    leftPid.targetValue = speed_control_speed;
    rightPid.targetValue = speed_control_speed;
    leftPid.kp = 125;
    leftPid.ki = 5;
    leftPid.kd = 10;
    rightPid.kp = 125;
    rightPid.ki = 5;
    rightPid.kd = 10;
    
    steer_actuator_right = 415; //740; //415;
    steer_actuator_middle = 479; //809; //479;
    steer_actuator_left = 543; //878; //543;
    
    pre_sight = 25;
    
    direction_control_kd = 0.2;
    direction_control_kpj = 0.025;
    direction_control_kpc = 0.000133;
    
    front_car = true;
}
