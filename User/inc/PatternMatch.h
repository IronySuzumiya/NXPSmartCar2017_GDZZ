#ifndef __PATTERN_MATCH_H
#define __PATTERN_MATCH_H

#include "root.h"

#include "Ring.h"
#include "CrossRoad.h"
#include "Curve.h"
#include "Barrier.h"

int16_t GetRoadType(void);
bool IsOutOfRoad(void);
bool IsStartLine(int16_t row);
bool IsStraightLine(void);
void StartlineAction(void);
bool IsRamp(void);

extern int16_t startLineWidth;
extern bool last_leader_car;
extern int16_t width__;

#endif
