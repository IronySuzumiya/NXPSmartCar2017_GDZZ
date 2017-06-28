#ifndef _MOTOR_H
#define _MOTOR_H

#include "root.h"

#define MOTOR_STOP MotorOut(0, 0)

void MotorInit(void);
void MotorOut(int16_t left, int16_t right);

extern bool motor_on;

#endif
