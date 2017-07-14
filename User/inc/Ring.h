#ifndef __RING_H
#define __RING_H

#include "root.h"

#if CAR_NO == 1
#define IsRingEnd IsRingEndFromRight
#define RingAction RingActionGoRight
#define RingEndAction RingEndActionFromRight
#elif CAR_NO == 2
#define IsRingEnd IsRingEndFromLeft
#define RingAction RingActionGoLeft
#define RingEndAction RingEndActionFromLeft
#endif

bool IsRing(void);
bool IsRingEndFromLeft(void);
bool IsRingEndFromRight(void);
void RingActionGoLeft(void);
void RingActionGoRight(void);
void RingEndActionFromLeft(void);
void RingEndActionFromRight(void);

extern int32_t ringDistance;
extern bool inRing;
extern bool ringEndDelay;

#endif
