#include "Action.h"
#include "DoubleCar.h"
#include "SpeedControl.h"
#include "ImgUtility.h"
#include "gpio.h"
#include "MainProc.h"
#include "ImgProc.h"

int32_t startDistance;
bool final;
bool waitForFinalPursueing;
bool finalPursueingFinished;
int32_t finalDistance;
int32_t wholeDistance;
bool startLineEnabled;
bool firstOvertakingFinished;
int32_t dashDistance;
bool sendOvertakingFinishedMsgLater;
int32_t sendOvertakingFinishedMsgLaterDistance;
int32_t sendOvertakingFinishedMsgLaterDistanceMax;
int32_t overtakingDistance;
int32_t overtakingDistanceMax;
bool afterCrossRoad;
int32_t afterCrossRoadDistance;
bool barrierOvertaking;
bool crossRoadOvertaking;
bool start_line;
bool out;
bool crossRoadActionEnabled;
bool onRamp;
int16_t rampDistance;
bool inStraightLine;
int16_t straightLineDistance;
int32_t preRingEndDistance;
bool preRingEnd;
bool hugeRing;

int16_t FirstOvertakingAction() {
    if(leader_car) {
        if(startDistance < 3000) {
            return IMG_COL / 2 - 28;
        } else {
            beingOvertaken = true;
            firstOvertakingFinished = true;
            leader_car = !leader_car;
            return IMG_COL / 2;
        }
    } else {
        if(startDistance < 8000) {
            return IMG_COL / 2 + 25;
        } else if(startDistance < 15000) {
            return IMG_COL / 2;
        } else {
            SendMessage(OVERTAKINGFINISHED);
            leader_car = !leader_car;
            firstOvertakingFinished = true;
            return IMG_COL / 2;
        }
    }
}

void FinalDashAction() {
    if(!leader_car && distanceBetweenTheTwoCars < avg_distance_between_the_two_cars - diff_distance_max) {
        finalPursueingFinished = true;
        SendMessage(DASH);
    }
    if(finalPursueingFinished) {
        if(leader_car && dashDistance > 17000) {
            stop = true;
        } else if(!leader_car && dashDistance > 17000) {
            stop = true;
        }
    }
}

int16_t CommonAction() {
    switch(GetRoadType()) {
        case HugeRing:
            hugeRing = true;
            BUZZLE_ON;
        case Ring:
//            if(double_car) {
//                if(leader_car && !inRing) {
//                    SendMessage(HOLD);
//                } else if(!leader_car && holding) {
//                    holding = false;
//                    holdingDistance = 0;
//                }
//            }
            inRing = true;
            if(ringOrder & (1 << ringOvertakingCnt)) {
                if(leader_car) {
                    RingActionGoRight();
                } else {
                    RingActionGoLeft();
                }
            } else {
                if(leader_car) {
                    RingActionGoLeft();
                } else {
                    RingActionGoRight();
                }
            }
//            BUZZLE_ON;
            break;
        case RingEnd:
            if(double_car && !overtaking) {
                if(leader_car) {
                    leader_car = !leader_car;
                    if(alreadyReceivedOvertakingFinished) {
                        alreadyReceivedOvertakingFinished = false;
                    } else {
                        beingOvertaken = true;
                    }
                } else {
                    sendOvertakingFinishedMsgLater = true;
                }
                overtaking = true;
            }
            BUZZLE_ON;
            if(ringOrder & (1 << ringOvertakingCnt)) {
                if(last_leader_car) {
                    return IMG_COL / 2 - 32;
                } else {
                    return IMG_COL / 2 + 26;
                }
            } else {
                if(last_leader_car) {
                    return IMG_COL / 2 + 26;
                } else {
                    return IMG_COL / 2 - 32;
                }
            }
        case CrossRoad:
            BUZZLE_ON;
//            inCrossRoad = true;
//            if(crossRoadActionEnabled) {
//                CrossRoadAction();
//            }
            break;
        case LeftCurve:
            LeftCurveAction();
            break;
        case RightCurve:
            RightCurveAction();
            break;
        case LeftBarrier:
            return IMG_COL / 2 - 20;
        case RightBarrier:
            return IMG_COL / 2 + 20;
        case DummyLeftBarrier:
            return IMG_COL / 2 - 35;
        case DummyRightBarrier:
            return IMG_COL / 2 + 35;
        case Ramp:
            onRamp = true;
            break;
        case Startline:
            StartlineAction();
            break;
    }
    return IMG_COL / 2;
}
