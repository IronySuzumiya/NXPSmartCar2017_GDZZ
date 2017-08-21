#include "DirectionControl.h"
#include "SteerActuator.h"
#include "ImgProc.h"
#include "uart.h"
#include "MainProc.h"
#include "Action.h"
#include "DoubleCar.h"

bool direction_control_on;
float directionAngle;
float direction_control_kd;
float direction_control_kpj;
float direction_control_kpc;

static int16_t DirectionErrorGet(int16_t* middleLine, int16_t expectMiddle);
static int16_t DirectionControlPID(int16_t dirError);

void DirectionControlProc(int16_t* middleLine, int16_t expectMiddle) {
    if(steer_actuator_on) {
        SteerActuatorOut(DirectionControlPID(DirectionErrorGet(middleLine, expectMiddle)));
    } else {
        SteerActuatorOut(steer_actuator_middle);
    }
}

int16_t DirectionErrorGet(int16_t* middleLine, int16_t expectMiddle) {
    float avgMiddle = 0;
    if(final && (!double_car || leader_car) && finalDistance < 4000) {
        for(int16_t i = 35; i < 45; ++i) {
            avgMiddle += middleLine[i];
        }
    } else {
        for(int16_t i = pre_sight - 5; i < pre_sight + 5; ++i) {
            avgMiddle += middleLine[i];
        }
    }
    avgMiddle /= 10;
    return expectMiddle - avgMiddle;
}

int16_t DirectionControlPID(int16_t error) {
    static int16_t lastError = 0;
    
	directionAngle = Min_f(direction_control_kpj + (error * error) * direction_control_kpc, 0.2) //0.28
        * error + direction_control_kd * (error - lastError);
    
    directionAngle = Limit_f(directionAngle, -14.4, 14.4);
    
    lastError = error;
    
    return directionAngle * 5.556 + steer_actuator_middle;
}
