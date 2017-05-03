#ifndef _STEER_ACTUATOR_H
#define _STEER_ACTUATOR_H

#include "root.h"

void SteerActuatorInit(void);
void SteerActuatorReset(void);
void SteerActuatorOut(int16_t out);

extern bool steer_actuator_on;
extern int16_t steer_actuator_left;
extern int16_t steer_actuator_middle;
extern int16_t steer_actuator_right;

#endif
