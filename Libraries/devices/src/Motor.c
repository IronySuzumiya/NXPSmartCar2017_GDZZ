#include "Motor.h"
#include "ftm.h"
#include "MainProc.h"

static void MotorInit(struct _motor *self);
static void MotorOut(int16_t left, int16_t right);

struct _motor motor = {
    MotorInit,
    MotorOut,
};

void MotorInit(struct _motor *self) {
	FTM_PWM_QuickInit(MOTOR_LEFT_FOR_MAP, kPWM_EdgeAligned, 10000);
	FTM_PWM_QuickInit(MOTOR_LEFT_BAK_MAP, kPWM_EdgeAligned, 10000);
	FTM_PWM_QuickInit(MOTOR_RIGHT_FOR_MAP, kPWM_EdgeAligned, 10000);
	FTM_PWM_QuickInit(MOTOR_RIGHT_BAK_MAP, kPWM_EdgeAligned, 10000);
    
    self->work = true;
    self->change_duty(0, 0);
}

void MotorOut(int16_t left, int16_t right) {
    left = util.control_in_range(left, -10000, 10000);
    right = util.control_in_range(right, -10000, 10000);
    if(left >= 0) {
        FTM_PWM_ChangeDuty(MOTOR_LEFT_PORT, MOTOR_LEFT_FOR_CHL, left);
        FTM_PWM_ChangeDuty(MOTOR_LEFT_PORT, MOTOR_LEFT_BAK_CHL, 0);
    } else {
        FTM_PWM_ChangeDuty(MOTOR_LEFT_PORT, MOTOR_LEFT_FOR_CHL, 0);
        FTM_PWM_ChangeDuty(MOTOR_LEFT_PORT, MOTOR_LEFT_BAK_CHL, -left);
    }
    if(right >= 0) {
        FTM_PWM_ChangeDuty(MOTOR_RIGHT_PORT, MOTOR_RIGHT_FOR_CHL, right);
        FTM_PWM_ChangeDuty(MOTOR_RIGHT_PORT, MOTOR_RIGHT_BAK_CHL, 0);
    } else {
        FTM_PWM_ChangeDuty(MOTOR_RIGHT_PORT, MOTOR_RIGHT_FOR_CHL, 0);
        FTM_PWM_ChangeDuty(MOTOR_RIGHT_PORT, MOTOR_RIGHT_BAK_CHL, -right);
    }
}
