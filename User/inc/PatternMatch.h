#ifndef __PATTERN_MATCH_H
#define __PATTERN_MATCH_H

#include "root.h"

#define IsRingEnd IsRingEndFromLeft

#define RingCompensate RingCompensateGoLeft

#define RingEndCompensate RingEndCompensateFromLeft

extern inline int16_t Abs(int16_t input) {
    return input >= 0 ? input : -input;
}

extern inline int16_t Min(int16_t a, int16_t b) {
    return a > b ? b : a;
}

extern inline int16_t Max(int16_t a, int16_t b) {
    return a > b ? a : b;
}

extern inline bool OpstSign(int16_t a, int16_t b) {
    return a * b < 0;
}

extern inline bool InRange(int16_t value, int16_t lbound, int16_t hbound) {
    return value > lbound && value < hbound;
}

int16_t GetRoadType(void);
void RingCompensateGoLeft(void);
void RingCompensateGoRight(void);
void RingEndCompensateFromLeft(void);
void RingEndCompensateFromRight(void);
void LeftCurveCompensate(void);
void RightCurveCompensate(void);
void CrossRoadCompensate(void);
void BarrierCompensate(void);
bool IsOutOfRoad(void);
bool StartLineJudge(int16_t row);
bool StraightLineJudge(void);

extern int32_t ringDistance;
extern int32_t crossRoadDistance;
extern bool inRing;
extern bool ringEndDelay;
extern bool inCrossRoad;
extern int32_t barrierDistance;
extern bool aroundBarrier;

#endif
