#ifndef __PATTERN_MATCH_H
#define __PATTERN_MATCH_H

#include "root.h"

road_type_type GetRoadType(void);
void RingCompensateGoLeft(void);
void RingCompensateGoRight(void);
void LeftCurveCompensate(void);
void RightCurveCompensate(void);
void CrossRoadCompensate(void);
bool StartLineJudge(int16_t row);
bool StraightLineJudge(void);

#endif
