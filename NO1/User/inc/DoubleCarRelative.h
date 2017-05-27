#ifndef _DOUBLE_CAR_RELATIVE_H
#define _DOUBLE_CAR_RELATIVE_H

#include "root.h"

extern inline float CalculateDistanceWithTime(uint32_t time) {
    return time * 0.034;
}

void DoubleCarRelativeInit(void);
void MessageEnqueue(uint8_t message);
void SendMessage(uint8_t message);

extern int16_t ultraSonicMissingCnt;
extern float distance;
extern uint32_t time;
extern bool front_car;
extern bool double_car;

#endif
