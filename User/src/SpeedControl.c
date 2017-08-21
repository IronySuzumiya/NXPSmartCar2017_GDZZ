#include "SpeedControl.h"
#include "Motor.h"
#include "Encoder.h"
#include "DirectionControl.h"
#include "DoubleCar.h"
#include "gpio.h"
#include "ImgUtility.h"
#include "ImgProc.h"

PID leftPid, rightPid;
int16_t leftSpeed, rightSpeed;
int16_t speed_control_sum_err_max;
bool speed_control_on;
int16_t speed_control_speed;
int16_t speed_control_acc;
int16_t speed_control_dec;
float reduction_ratio;
float differential_ratio;
bool stop;
int16_t speedAroundBarrier;
int16_t speedInRing;

static int16_t SpeedControlPID(PID *pid);
static void SpeedControlFilter(int16_t newValue, PID* pid);

void SpeedControlProc(int16_t leftSpeed, int16_t rightSpeed) {
    SpeedControlFilter(leftSpeed, &leftPid);
    SpeedControlFilter(rightSpeed, &rightPid);
    if(motor_on) {
        MotorOut(SpeedControlPID(&leftPid), SpeedControlPID(&rightPid));
    } else {
        MOTOR_STOP;
    }
}

void SpeedTargetSet(int16_t speed, bool diff) {
    if(double_car && !leader_car && speed != 0) {
        if(TOO_FAR) {
            speed += speed_control_acc;
        } else if(TOO_CLOSE) {
            speed -= speed_control_dec;
        }
    }
    if(!diff || speed == 0) {
        leftPid.targetValue = rightPid.targetValue = speed;
    } else {
        if(directionAngle > 0) {
            speed -= reduction_ratio * directionAngle;
            leftPid.targetValue = speed;
            rightPid.targetValue = speed * (differential_ratio * directionAngle + 1);
        } else {
            speed += reduction_ratio * directionAngle;
            rightPid.targetValue = speed;
            leftPid.targetValue = speed * (differential_ratio * (-directionAngle) + 1);
        }
    }
}

int16_t SpeedControlPID(PID *pid) {
	int16_t error;
    double pValue, iValue, dValue;
    
	error = pid->targetValue - pid->currentValue;
    
    // ugly implementation though
    if(beingOvertaken || stop) {
        if(error < -3 || error > 3) {
            pValue = 110 * (error - pid->lastError);
            iValue = 20 * error;
            dValue = 10 * (error - 2 * pid->lastError + pid->prevError);
        } else {
            pValue = iValue = dValue = 0;
        }
    } else {
        pValue = pid->kp * (error - pid->lastError);
        iValue = pid->ki * error;
        dValue = pid->kd * (error - 2 * pid->lastError + pid->prevError);
    }
    pid->output += pValue + iValue + dValue;
    if(pid->output > 10000) {
        pid->output = 10000;
    }
    
    pid->prevError = pid->lastError;
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
