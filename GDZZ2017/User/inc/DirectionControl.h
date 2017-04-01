#ifndef _DIRECTION_CONTROL_H
#define _DIRECTION_CONTROL_H

#include "root.h"

void DirectionControlProc(int16_t* middleLine);

extern double directionAngle;
extern double direction_control_kd;
extern double direction_control_kpj;
extern double direction_control_kpc;
extern double direction_control_angle_max;

#endif
