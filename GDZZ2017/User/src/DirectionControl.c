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
    for(int16_t i = pre_sight - slope_sensitivity; i < pre_sight + slope_sensitivity + 2; ++i) {
        avgMiddle += middleLine[i];
    }
    avgMiddle /= slope_sensitivity * 2 + 2;
    return IMG_COL / 2 - avgMiddle;
}

int16_t DirectionControlPID(int16_t error) {
//    static char buf[50];
//    static int cnt = 0;
    
    static int16_t lastError = 0;
    float kp = direction_control_kpj + (error * error) * direction_control_kpc;
    
	directionAngle = kp * error + direction_control_kd * (error - lastError);
    
//    if(cnt > 50) {
//        sprintf(buf, "error=%d, d=%f\r\n", error, directionAngle);
//        UART_printf(DATACOMM_IMG_TRANS_CHL, buf);
//        cnt = 0;
//    }
//    ++cnt;
    
	if(directionAngle > steer_actuator_left - steer_actuator_middle) {
		directionAngle = steer_actuator_left - steer_actuator_middle;
    } else if(directionAngle < steer_actuator_right - steer_actuator_middle) {
		directionAngle = steer_actuator_right - steer_actuator_middle;
    }
    
    lastError = error;
    
    return directionAngle + steer_actuator_middle;
}
