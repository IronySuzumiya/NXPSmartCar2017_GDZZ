#include "Action.h"
#include "DoubleCar.h"
#include "SpeedControl.h"
#include "ImgUtility.h"

int32_t startDistance;
bool final;
bool finalOvertakingFinished;
bool finalPursueingFinished;
int32_t finalDistance;
int32_t wholeDistance;
bool startLineEnabled;
bool firstOvertakingFinished;
bool goAlongLeft;
int32_t dashDistance;

int16_t FirstOvertakingAction() {
    if(leader_car) {
        if(startDistance < 20000) {
            return IMG_COL / 2 - 40;
        } else {
            waitForOvertaking = true;
            aroundOvertaking = true;
            firstOvertakingFinished = true;
        }
    } else {
        if(startDistance < 30000) {
            return IMG_COL / 2 + 30;
        } else if(startDistance > 60000) {
            overtaking = true;
            SendMessage(MOVE_RIGHT_NOW);
            aroundOvertaking = true;
            firstOvertakingFinished = true;
        }
    }
    return IMG_COL / 2;
}

int16_t FinalOvertakingAction() {
    if(leader_car) {
        if(finalDistance < 11000) {
            finalOvertakingFinished = true;
            return IMG_COL / 2 - 48;
        } else if(finalDistance < 25000 && goAlongLeft) {
            return IMG_COL / 2 + 32;
        } else if(finalDistance < 50000) {
            if(finalOvertakingFinished) {
                finalOvertakingFinished = false;
                waitForOvertaking = true;
                aroundOvertaking = true;
            }
        } else {
            stop = true;
        }
    } else {
        if(distanceBetweenTheTwoCars < 25000 && goAlongLeft) {
            return IMG_COL / 2 + 32;
        } else if(distanceBetweenTheTwoCars < 40000) {
            if(goAlongLeft) {
                return IMG_COL / 2 + 32;
            }
            if(!aroundOvertaking) {
                overtaking = true;
                SendMessage(OVERTAKING);
                aroundOvertaking = true;
            }
        } else {
            stop = true;
        }
    }
    return IMG_COL / 2;
}

void FinalDashAction() {
    if(!leader_car && pursueing && distanceBetweenTheTwoCars < 80) {
        finalPursueingFinished = true;
        pursueing = false;
        SendMessage(DASH);
    }
    if(finalPursueingFinished && dashDistance > 70000) {
        stop = true;
    }
}

int16_t CommonAction() {
    switch(GetRoadType()) {
        case Ring:
            inRing = true;
            RingAction();
            break;
        case RingEnd:
            if(double_car && !aroundOvertaking) {
                if(leader_car) {
                    waitForOvertaking = true;
                } else if(!overtaking) {
                    overtaking = true;
                    SendMessage(OVERTAKING);
                }
                aroundOvertaking = true;
            }
            RingEndAction();
            break;
        case CrossRoad:
            inCrossRoad = true;
            crossRoadDistance = 0;
            if(double_car && leader_car) {
                SendMessage(CROSS_ROAD);
            }
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
    }
    return IMG_COL / 2;
}
