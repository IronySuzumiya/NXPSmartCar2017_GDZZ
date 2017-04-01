#ifndef _MOTOR_H
#define _MOTOR_H

#define MOTOR

#include "root.h"

void MotorInit(void);
void MotorOut(int16_t left, int16_t right);

extern bool motor_on;

#endif
