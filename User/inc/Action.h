#ifndef __ACTION_H
#define __ACTION_H

#include "root.h"

int16_t FirstOvertakingAction(void);
int16_t FinalOvertakingAction(void);
void FinalDashAction(void);
int16_t CommonAction(void);

extern int32_t startDistance;
extern bool final;
extern bool finalOvertakingFinished;
extern bool finalPursueingFinished;
extern int32_t finalDistance;
extern int32_t wholeDistance;
extern bool startLineEnabled;
extern bool firstOvertakingFinished;
extern bool goAlongLeft;
extern int32_t dashDistance;

#endif
