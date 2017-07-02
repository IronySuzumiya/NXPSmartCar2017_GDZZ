#include "SpeedControl.h"
#include "Motor.h"
#include "Encoder.h"
#include "DirectionControl.h"
#include "DoubleCarRelative.h"
#include "gpio.h"
#include "ImgProc.h"

PID leftPid, rightPid;
int16_t leftSpeed, rightSpeed;
int16_t speed_control_sum_err_max;
bool speed_control_on;
int16_t speed_control_speed;
int16_t speed_control_acc;
int16_t speed_control_dec;

#if defined(DYNAMIC_PRESIGHT) && !defined(PURSUEING_DEPENDING_PRESIGHT)
static int16_t GetPresight(int16_t speed);
#endif
static int16_t SpeedControlAcc(int16_t speed);
static int16_t SpeedControlDec(int16_t speed);
static int16_t SpeedControlPID(PID *pid);
static void SpeedControlFilter(int16_t newValue, PID* pid);

void SpeedControlProc(int16_t leftSpeed, int16_t rightSpeed) {
    SpeedControlFilter(leftSpeed, &leftPid);
    SpeedControlFilter(rightSpeed, &rightPid);
    MotorOut(SpeedControlPID(&leftPid), SpeedControlPID(&rightPid));
}

void SpeedTargetSet(int16_t speed, bool diff) {
    #ifdef DOUBLE_CAR
        if(speed != 0) {
            if(front_car) {
                if(!firstOvertaking || pursue) {
                    speed = SpeedControlDec(speed);
                }
                #if defined(DYNAMIC_PRESIGHT) && defined(PURSUEING_DEPENDING_PRESIGHT)
                    else {
                        pre_sight = pre_sight_default;
                    }
                #endif
            } else {
                if(!firstOvertaking || pursue) {
                    speed = SpeedControlAcc(speed);
                } else if(TOO_FAR) {
                    speed = SpeedControlAcc(speed);
                } else if(TOO_CLOSE) {
                    speed = SpeedControlDec(speed);
                }
                #if defined(DYNAMIC_PRESIGHT) && defined(PURSUEING_DEPENDING_PRESIGHT)
                    else {
                        pre_sight = pre_sight_default;
                    }
                #endif
            }
        }
    #endif
    
    if(!diff) {
        leftPid.targetValue = rightPid.targetValue = speed;
    } else {
        if(directionAngle > 0) {
             speed -= 2.6 * directionAngle;
             leftPid.targetValue = speed;
             rightPid.targetValue = speed * (0.031 * directionAngle + 1);
        } else {
             speed += 2.6 * directionAngle;
             rightPid.targetValue = speed;
             leftPid.targetValue = speed * (0.031 * (-directionAngle) + 1);
        }
    }
    #if defined(DYNAMIC_PRESIGHT) && !defined(PURSUEING_DEPENDING_PRESIGHT)
        pre_sight = GetPresight(speed);
    #endif
}

int16_t SpeedControlAcc(int16_t speed) {
    speed = speed + speed_control_acc;
    #if defined(DYNAMIC_PRESIGHT) && defined(PURSUEING_DEPENDING_PRESIGHT)
        pre_sight = pre_sight_default + 2;
    #endif
    return speed;
}

int16_t SpeedControlDec(int16_t speed) {
    speed = speed - speed_control_dec;
    #if defined(DYNAMIC_PRESIGHT) && defined(PURSUEING_DEPENDING_PRESIGHT)
        pre_sight = pre_sight_default - 2;
    #endif
    return speed;
}

int16_t GetPresight(int16_t speed) {
    return speed < 85 ? 20 : speed < 90 ? 22 : speed < 95 ? 23 : speed < 100 ? 24 :
        speed < 105 ? 26 : speed < 110 ? 27 : speed < 115 ? 28 : 30;
}

int16_t SpeedControlPID(PID *pid) {
	int16_t error;
    double pValue, iValue, dValue;
    
	error = pid->targetValue - pid->currentValue;
    
    #ifdef INC_PID
        pValue = pid->kp * (error - pid->lastError);
        iValue = pid->ki * error;
        dValue = pid->kd * (error - 2 * pid->lastError + pid->prevError);
        pid->prevError = pid->lastError;
        pid->output += pValue + iValue + dValue;
        if(pid->output > 10000) {
            pid->output = 10000;
        }
    #else
        pid->sumError += error;
        if(pid->sumError > speed_control_sum_err_max) {
            pid->sumError = speed_control_sum_err_max;
        } else if(pid->sumError < -speed_control_sum_err_max) {
            pid->sumError = -speed_control_sum_err_max;
        }
        pValue = pid->kp * error;
        iValue = pid->ki * pid->sumError;
        dValue = pid->kd * (error - pid->lastError);
        pid->output = pValue + iValue + dValue;
    #endif
    
	pid->lastError = error;
    
	return (int16_t)pid->output;
}

void SpeedControlFilter(int16_t newValue, PID* pid) {
    uint8_t cnt;
    pid->valueBuf[pid->cursor] = newValue;
    pid->cursor++;
    if(pid->cursor / SPEED_BUF_SIZE) {
        pid->cursor = 0;
    }
    pid->currentValue = pid->valueBuf[0];
    for(cnt = 1; cnt < SPEED_BUF_SIZE; cnt++) {
        pid->currentValue += pid->valueBuf[cnt];
        pid->currentValue /= 2;
    }
}
