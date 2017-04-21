#include "DirectionControl.h"
#include "SteerActuator.h"
#include "ImgProc.h"
#include "uart.h"

double directionAngle;
double direction_control_kd;
double direction_control_kpj;
double direction_control_kpc;
double direction_control_angle_max;

static int16_t DirectionErrorGet(int16_t* middleLine);
static int16_t DirectionControlPID(int16_t dirError);

void DirectionControlProc(int16_t* middleLine) {
    SteerActuatorOut(DirectionControlPID(DirectionErrorGet(middleLine)));
}

int16_t DirectionErrorGet(int16_t* middleLine) {
    float avgMiddle = 0;
    for(int16_t i = pre_sight - slope_sensitivity; i < pre_sight + slope_sensitivity; ++i) {
        avgMiddle += middleLine[i];
    }
    avgMiddle /= slope_sensitivity * 2;
    return IMG_COL / 2 - avgMiddle;
}

int16_t DirectionControlPID(int16_t error) {
    static int16_t lastError = 0;
    float kp = direction_control_kpj + (error * error) * direction_control_kpc;
    if(kp > 0.24)
    {
        kp = 0.24;
    }
	directionAngle = kp * error + direction_control_kd * (error - lastError);
    
	if(directionAngle > 14.4) {
		directionAngle = 14.4;
    } else if(directionAngle < -14.4) {
		directionAngle = -14.4;
    }
    
    lastError = error;
    
    return directionAngle * 255.556 + steer_actuator_middle;
}
