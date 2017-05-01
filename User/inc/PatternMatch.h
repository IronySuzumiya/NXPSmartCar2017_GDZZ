#ifndef __PATTERN_MATCH_H
#define __PATTERN_MATCH_H

#include "root.h"

bool OutOfRoadJudge(int16_t row);
void OutOfRoadJudgeExecute(void);

int16_t GetRoadType(void);
void RingCompensateGoLeft(void);
void RingCompensateGoRight(void);
void RingEndCompensateFromLeft(void);
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
