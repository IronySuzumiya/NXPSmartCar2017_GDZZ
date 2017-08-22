#ifndef _DIRECTION_CONTROL_H
#define _DIRECTION_CONTROL_H

#include "root.h"

void DirectionControlProc(int16_t* middleLine, int16_t expectMiddle);

extern bool direction_control_on;
extern float directionAngle;
extern float direction_control_kd;
extern float direction_control_kpj;
extern float direction_control_kpc;

extern bool dirlocked;

#endif
