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
extern bool final_overtaking;
extern int16_t ultraSonicMissingCnt;
extern float distanceBetweenTheTwoCars;
extern uint32_t time;
extern bool leader_car;
extern bool pursueing;
extern int32_t avg_distance_between_the_two_cars;
extern int32_t diff_distance_max;
extern bool waitForOvertaking;
extern bool overtaking;
extern bool aroundOvertaking;
extern int32_t waitForOvertakingTimeMax;
extern int32_t overtakingTime;
extern int32_t aroundOvertakingTimeMax;
extern bool holding;
extern int32_t holdingDistance;

#endif
