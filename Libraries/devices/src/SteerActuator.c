#include "SteerActuator.h"
#include "ftm.h"

static void SteerActuatorInit(struct _steer *self);
static void SteerActuatorOut(int16_t out);

struct _steer steer = {
    SteerActuatorInit,
    SteerActuatorOut,
    #if CAR_NO == 1
    750
    #elif CAR_NO == 2
    755
    #else
    #error invalid configuration
    0
    #endif
};

void SteerActuatorInit(struct _steer *self) {
    FTM_PWM_QuickInit(STEER_ACTUATOR_MAP, kPWM_EdgeAligned, 50);
    self->work = true;
    self->change_duty(self->middle_duty);
}

void SteerActuatorOut(int16_t out) {
    FTM_PWM_ChangeDuty(STEER_ACTUATOR_PORT, STEER_ACTUATOR_CHL, out);
}
