#ifndef __PATTERN_MATCH_H
#define __PATTERN_MATCH_H

#include "root.h"

#define MAYBE_BARRIER (!inRing && !ringEndDelay ? WhichBarrier() : Unknown)

extern inline float Abs(float input) {
    return input >= 0 ? input : -input;
}

extern inline float Min(float a, float b) {
    return a > b ? b : a;
}

extern inline float Max(float a, float b) {
    return a > b ? a : b;
}

extern inline bool OpstSign(int16_t a, int16_t b) {
    return (a & 0x8000) ^ (b & 0x8000);
}

extern inline bool InRange(int16_t value, int16_t lbound, int16_t hbound) {
    return value > lbound && value < hbound;
}

int16_t GetRoadType(void);
bool OutOfRoadJudge(void);
void RingCompensateGoLeft(void);
void RingCompensateGoRight(void);
void RingEndCompensateFromLeft(void);
void RingEndCompensateFromRight(void);
void LeftCurveCompensate(void);
void RightCurveCompensate(void);
void CrossRoadCompensate(void);
void BarrierCompensate(void);
bool StartLineJudge(int16_t row);
bool StraightLineJudge(void);

extern int32_t ringDistance;
extern bool inRing;
extern bool ringEndDelay;
extern int32_t barrierDistance;
extern bool aroundBarrier;

#endif
