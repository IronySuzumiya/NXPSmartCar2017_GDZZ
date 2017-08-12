#ifndef _DOUBLE_CAR_RELATIVE_H
#define _DOUBLE_CAR_RELATIVE_H

#include "root.h"

extern inline float CalculateDistanceWithTime(uint32_t time) {
    return time * 0.034;
}

void DoubleCarInit(void);
void MessageEnqueue(uint8_t message);
void SendMessage(uint8_t message);

extern bool double_car;
extern int16_t ultraSonicMissingCnt;
extern float distanceBetweenTheTwoCars;
extern uint32_t time;
extern bool leader_car;
extern int32_t avg_distance_between_the_two_cars;
extern int32_t diff_distance_max;
extern bool holding;
extern int32_t holdingDistance;
extern bool overtaking;
extern bool beingOvertaken;
extern bool alreadyReceivedOvertakingFinished;
extern bool rampOvertaking;

#endif
