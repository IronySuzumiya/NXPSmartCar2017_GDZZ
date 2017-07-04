#include "Motor.h"
#include "ftm.h"
#include "MainProc.h"

bool motor_on;

void MotorInit() {
	FTM_PWM_QuickInit(MOTOR_LEFT_FOR_MAP, kPWM_EdgeAligned, 10000);
	FTM_PWM_QuickInit(MOTOR_LEFT_BAK_MAP, kPWM_EdgeAligned, 10000);
	FTM_PWM_QuickInit(MOTOR_RIGHT_FOR_MAP, kPWM_EdgeAligned, 10000);
	FTM_PWM_QuickInit(MOTOR_RIGHT_BAK_MAP, kPWM_EdgeAligned, 10000);
    
    MOTOR_STOP;
}

void MotorOut(int16_t left, int16_t right) {
    left = Limit(left, -10000, 10000);
    right = Limit(right, -10000, 10000);
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
