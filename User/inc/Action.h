#ifndef __ACTION_H
#define __ACTION_H

#include "root.h"

int16_t FirstOvertakingAction(void);
int16_t SecondOvertakingAction(void);
int16_t FinalOvertakingAction(void);
void FinalDashAction(void);
int16_t CommonAction(void);

extern int32_t startDistance;
extern bool final;
extern bool finalPursueingFinished;
extern int32_t finalDistance;
extern int32_t wholeDistance;
extern bool startLineEnabled;
extern bool firstOvertakingFinished;
extern int32_t dashDistance;
extern bool sendOvertakingFinishedMsgLater;
extern int32_t sendOvertakingFinishedMsgLaterDistance;
extern int32_t sendOvertakingFinishedMsgLaterDistanceMax;
extern int32_t overtakingDistance;
extern int32_t overtakingDistanceMax;
extern bool afterCrossRoad;
extern int32_t afterCrossRoadDistance;
extern bool barrierOvertaking;
extern bool crossRoadOvertaking;
extern bool start_line;
extern bool out;

#endif
