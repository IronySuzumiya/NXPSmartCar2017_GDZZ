#include "MainProc.h"
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
    
    ImgProcInit();
    
    NVICInit();
    
    BuzzleInit();
    
    if(double_car)
        DoubleCarRelativeInit();
    
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
    EncoderGet(&leftSpeed, &rightSpeed);
    if(speed_control_on) {
        SpeedControlProc(leftSpeed, rightSpeed);
    }
}

static void SwitchAndParamLoad(void) {
    #warning "All these variables should be loaded from SD Card in the future!"
    motor_on = true;
    encoder_on = true;
    speed_control_on = true;
    direction_control_on = true;
    steer_actuator_on = true;
    img_trans_on = true;
    state_trans_on = false;
    mode_switch_on = false;
    use_inc_pid = false;
    use_ftm_qd = false;
    double_car = false;
    
    speed_control_speed = 108;
    speed_control_sum_err_max = 1000;
    
    speed_control_acc_speed = 90;
    speed_control_dec_speed = 50;
    
    leftPid.targetValue = speed_control_speed;
    rightPid.targetValue = speed_control_speed;
    leftPid.kp = 120;
    leftPid.ki = 5;
    leftPid.kd = 10;
    rightPid.kp = 120;
    rightPid.ki = 5;
    rightPid.kd = 10;
    
    steer_actuator_right = 415;
    steer_actuator_middle = 479;
    steer_actuator_left = 543;
    
    pre_sight = 28;
    
    img_border_scan_compensation = 15;
    wide_road_size = 80;
    curve_sensitivity = 5;
    slope_sensitivity = 3;
    inflexion_sensitivity = 2;
    cross_road_size = 200;
    straight_road_sensitivity = 10;
    straight_road_middle_area_cnt_min = 38;
    startline_sensitivity = 6;
    startline_black_tape_num = 7;
    mini_s_visual_field = 35;
    mini_s_sensitivity = 50;
    direction_control_kd = 0.2;
    direction_control_kpj = 0.02;
    direction_control_kpc = 0.000133;
    
    speed_control_curves_speed_gain = 0.78;
    speed_control_curves_differential_gain = 0.0020;//0.0022;
    
    front_car = true;
}
