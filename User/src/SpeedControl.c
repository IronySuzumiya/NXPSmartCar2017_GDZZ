#include "SpeedControl.h"
#include "Motor.h"
#include "Encoder.h"
#include "DirectionControl.h"
#include "DoubleCarRelative.h"
#include "gpio.h"

PID leftPid, rightPid;
int16_t leftSpeed, rightSpeed;
int16_t speed_control_sum_err_max;
bool speed_control_on;
int16_t speed_control_speed;
int16_t speed_control_acc_speed;
int16_t speed_control_dec_speed;
double speed_control_curves_speed_gain;
double speed_control_curves_differential_gain;

static int16_t current_speed;

static int16_t SpeedControlPID(PID *pid);
static void SpeedControlFilter(int16_t newValue, PID* pid);

void SpeedControlProc(int16_t leftSpeed, int16_t rightSpeed) {
    SpeedControlFilter(leftSpeed, &leftPid);
    SpeedControlFilter(rightSpeed, &rightPid);
    MotorOut(SpeedControlPID(&leftPid), SpeedControlPID(&rightPid));
}

void SpeedTargetSet(uint16_t imgProcFlag) {
//    static int16_t slowDownCnt = 0;
//    static bool slowDown = false;
    
    if(double_car) {
        if(distance > AVG_DISTANCE_BETWEEN + DIFF_DISTANCE_MAX) {
            current_speed = speed_control_acc_speed;
        } else if(distance < AVG_DISTANCE_BETWEEN - DIFF_DISTANCE_MAX) {
            current_speed = speed_control_dec_speed;
        } else {
            current_speed = speed_control_speed;
        }
    } else {
        current_speed = speed_control_speed;
    }
    
    /*if(slowDown) {
        ++slowDownCnt;
        leftPid.targetValue = rightPid.targetValue = 60;
        if(slowDownCnt > 50) {
            slowDown = false;
            slowDownCnt = 0;
        }
    } else */if(imgProcFlag & STRAIGHT_ROAD) {
        leftPid.targetValue = rightPid.targetValue = current_speed + 12;
//    } else if(imgProcFlag & RING) {
//        slowDown = true;
    } else {
        int16_t tmpSpeed;
        if(directionAngle > 0)
        {
             tmpSpeed = current_speed - 2.6 * directionAngle;
             leftPid.targetValue = tmpSpeed;
             rightPid.targetValue = tmpSpeed * (0.035 * directionAngle + 1);//0.342
        }
        else
        {
             tmpSpeed = current_speed + 2.6 * directionAngle;
             rightPid.targetValue = tmpSpeed;
             leftPid.targetValue = tmpSpeed * (0.035 * (-directionAngle) + 1);
        }
    }
}

int16_t SpeedControlPID(PID *pid) {
	int16_t error;
    double pValue, iValue, dValue;
    
	error = pid->targetValue - pid->currentValue;
    
    if(use_inc_pid) {
        pValue = pid->kp * (error - pid->lastError);
        iValue = pid->ki * error;
        dValue = pid->kd * (error - 2 * pid->lastError + pid->prevError);
        pid->prevError = pid->lastError;
        pid->output += pValue + iValue + dValue;
    } else {
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
    }
    
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
