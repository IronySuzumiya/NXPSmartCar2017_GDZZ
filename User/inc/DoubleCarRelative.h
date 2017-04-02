#ifndef _DOUBLE_CAR_RELATIVE_H
#define _DOUBLE_CAR_RELATIVE_H

#include "root.h"

#define CalculateDistanceWithTime(time) ((time) * 0.034)

void DoubleCarRelativeInit(void);
void DoubleCarMessageRecv(uint16_t byte);

extern float distance;
extern uint32_t time;
extern bool front_car;
extern bool double_car;

#endif
