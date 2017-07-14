#include "Action.h"
#include "DoubleCar.h"
#include "SpeedControl.h"
#include "ImgUtility.h"
#include "gpio.h"
#include "MainProc.h"

int32_t startDistance;
bool final;
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
        case Ring:
            if(double_car) {
                if(leader_car && !inRing) {
                    SendMessage(HOLD);
                } else if(holding) {
                    holding = false;
                    holdingDistance = 0;
                }
            }
            inRing = true;
            RingAction();
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
            #if CAR_NO == 1
            return IMG_COL / 2 - 22;
            #elif CAR_NO == 2
            return IMG_COL / 2 + 22;
            #endif
        case CrossRoad:
            inCrossRoad = true;
            CrossRoadAction();
            break;
        case LeftCurve:
            LeftCurveAction();
            break;
        case RightCurve:
            RightCurveAction();
            break;
        case LeftBarrier:
            return IMG_COL / 2 - 22;
        case RightBarrier:
            return IMG_COL / 2 + 22;
        case DummyLeftBarrier:
            return IMG_COL / 2 - 40;
        case DummyRightBarrier:
            return IMG_COL / 2 + 40;
    }
    return IMG_COL / 2;
}
