#include "SteerActuator.h"
#include "ftm.h"

bool steer_actuator_on;
int16_t steer_actuator_left;
int16_t steer_actuator_middle;
int16_t steer_actuator_right;

void SteerActuatorInit() {
    FTM_PWM_QuickInit(STEER_ACTUATOR_MAP, kPWM_EdgeAligned, 50);
    FTM_PWM_ChangeDuty(STEER_ACTUATOR_PORT, STEER_ACTUATOR_CHL, steer_actuator_middle);
}

void SteerActuatorReset() {
    SteerActuatorOut(steer_actuator_middle);
}

void SteerActuatorOut(int16_t out) {
    FTM_PWM_ChangeDuty(STEER_ACTUATOR_PORT, STEER_ACTUATOR_CHL, out);
}
