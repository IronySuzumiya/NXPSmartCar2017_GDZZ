#include "Action.h"
#include "DoubleCar.h"
#include "SpeedControl.h"
#include "ImgUtility.h"
#include "gpio.h"
#include "MainProc.h"

int32_t startDistance;
int32_t secondDistance;
bool final;
bool finalOvertakingFinished;
bool finalPursueingFinished;
int32_t finalDistance;
int32_t wholeDistance;
bool startLineEnabled;
bool firstOvertakingFinished;
bool secondOvertakingFinished;
bool goAlongLeft;
int32_t dashDistance;

int16_t FirstOvertakingAction() {
    if(leader_car) {
        if(startDistance < 6000) {
            return IMG_COL / 2 - 40;
        } else {
            waitForOvertaking = true;
            aroundOvertaking = true;
            firstOvertakingFinished = true;
            return IMG_COL / 2 - (secondOvertakingFinished ? 0 : 40);
        }
    } else {
        if(startDistance < 8000) {
            return IMG_COL / 2 + 30;
        } else if(startDistance < 14000) {
            return IMG_COL / 2 + (secondOvertakingFinished ? 0 : 30);
        } else {
            overtaking = true;
            SendMessage(MOVE_RIGHT_NOW);
            aroundOvertaking = true;
            overtakingCnt = overtakingTime;
            aroundOvertakingCnt = aroundOvertakingTimeMax;
            firstOvertakingFinished = true;
            return IMG_COL / 2 + (secondOvertakingFinished ? 0 : 30);
        }
    }
}

int16_t SecondOvertakingAction() {
    if(leader_car) {
        if(secondDistance < 1000) {
            return IMG_COL / 2 - 40;
        } else {
            waitForOvertaking = true;
            aroundOvertaking = true;
            secondOvertakingFinished = true;
            return IMG_COL / 2 - (secondOvertakingFinished ? 0 : 40);
        }
    } else {
        if(secondDistance < 3000) {
            return IMG_COL / 2 + 30;
        } else if(secondDistance < 9000) {
            return IMG_COL / 2 + (secondOvertakingFinished ? 0 : 30);
        } else {
            overtaking = true;
            SendMessage(MOVE_RIGHT_NOW);
            aroundOvertaking = true;
            overtakingCnt = overtakingTime;
            aroundOvertakingCnt = aroundOvertakingTimeMax;
            secondOvertakingFinished = true;
            return IMG_COL / 2 + (secondOvertakingFinished ? 0 : 30);
        }
    }
}

int16_t FinalOvertakingAction() {
    if(leader_car) {
        if(finalDistance < 2200) {
            finalOvertakingFinished = true;
            return IMG_COL / 2 - 48;
        } else if(finalDistance < 5000 && goAlongLeft) {
            return IMG_COL / 2 + 32;
        } else if(finalDistance < 10000) {
            if(finalOvertakingFinished) {
                finalOvertakingFinished = false;
                waitForOvertaking = true;
                aroundOvertaking = true;
            }
        } else {
            stop = true;
        }
    } else {
        if(distanceBetweenTheTwoCars < 5000 && goAlongLeft) {
            return IMG_COL / 2 + 32;
        } else if(distanceBetweenTheTwoCars < 8000) {
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
    if(!leader_car && pursueing && distanceBetweenTheTwoCars < avg_distance_between_the_two_cars - 2 * diff_distance_max) {
        finalPursueingFinished = true;
        pursueing = false;
        SendMessage(DASH);
    }
    if(finalPursueingFinished && dashDistance > 14000) {
        stop = true;
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
            if(double_car && !aroundOvertaking) {
                if(leader_car) {
                    waitForOvertaking = true;
                } else if(!overtaking) {
                    overtaking = true;
                    SendMessage(OVERTAKING);
                }
                aroundOvertaking = true;
            }
            #if CAR_NO == 1
            return IMG_COL / 2 - 22;
            #elif CAR_NO == 2
            return IMG_COL / 2 + 22;
            #endif
        case CrossRoad:
            inCrossRoad = true;
            crossRoadDistance = 0;
            if(double_car && leader_car) {
                SendMessage(CROSS_ROAD);
            }
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
    }
    return IMG_COL / 2;
}
