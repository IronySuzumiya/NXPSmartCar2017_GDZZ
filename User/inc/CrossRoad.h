#ifndef __CROSS_ROAD_H
#define __CROSS_ROAD_H

#include "root.h"

bool IsCrossRoad(void);
void CrossRoadAction(void);

extern int32_t crossRoadDistance;
extern int32_t crossRoadDistanceMax;
extern bool inCrossRoad;
extern int32_t skippingFirstCrossRoadDistance;
extern int32_t skippingFirstCrossRoadDistanceMax;
extern bool skippingFirstCrossRoad;
extern bool firstCrossRoadSkipped;

#endif
