#include "PatternMatch.h"
#include "ImgProc.h"
#include "DoubleCar.h"
#include "MainProc.h"
#include "Action.h"
#include "Gyro.h"
#include "gpio.h"
#include "SpeedControl.h"
#include "Motor.h"
#include "ModeSwitch.h"

static bool placeholder;

int16_t startLineWidth;
bool last_leader_car;
int16_t width__;

int16_t GetRoadType() {
    if(ringEndDelay) {
        if(ringDistance < 3200) {
            return RingEnd;
        } else {
            hugeRing = false;
            ringEndDelay = false;
            ringDistance = 0;
            preRingEnd = true;
        }
    } else if(inRing) {
        if(ringDistance > 50000) {
            hugeRing = false;
            inRing = false;
            ringDistance = 0;
        } else if((hugeRing ? ringDistance > 9000 : ringDistance > 4000)
            && (ringOrder & (1 << ringOvertakingCnt) ? (leader_car ? IsRingEndFromRight() : IsRingEndFromLeft()) :
            (leader_car ? IsRingEndFromLeft() : IsRingEndFromRight()))) {
            last_leader_car = leader_car;
            inRing = false;
            ringDistance = 0;
            ringEndDelay = true;
            return RingEnd;
        } else if(hugeRing ? (ringDistance < 2100) : (ringDistance > 450 && ringDistance < 1800)) {
            return Ring;
        }
    } else if(aroundBarrier) {
        if(double_car) {
            if(barrierOvertakingEnabled && barrierOvertakingCnt < barrierOvertakingCntMax) {
                if(!barrierDoubleOvertakingEnabled) {
                    if(leader_car) {
                        if(barrierDistance < 5500) {
                            if(!placeholder) {
                                placeholder = true;
                                barrierOvertaking = true;
                                SendMessage(BARIOVERTAKING);
                            }
                            along = (barrierType == LeftBarrier ? AlongRightRoad : AlongLeftRoad);
                        } else if(barrierDistance < 6500) {
                            placeholder = false;
                            lastAlong = along;
                            along = (barrierType == LeftBarrier ? AlongLeftBorder : AlongRightBorder);
                        } else if(barrierDistance < 11000) {
                            lastAlong = along;
                            along = (barrierType == LeftBarrier ? AlongLeftRoad : AlongRightRoad);
                        } else {
                            barrierDistance = 0;
                            aroundBarrier = false;
                            leader_car = !leader_car;
                            beingOvertaken = true;
                            placeholder = false;
                            barrierOvertaking = false;
                            ++barrierOvertakingCnt;
                        }
                    } else if(barrierOvertaking) {
                        if(barrierDistance < 16000) {
                            along = (barrierType == LeftBarrier ? AlongRightRoad : AlongLeftRoad);
                        } else if(barrierDistance < 20000) {
                            along = AsUsual;
                        } else {
                            barrierDistance = 0;
                            aroundBarrier = false;
                            leader_car = !leader_car;
                            SendMessage(OVERTAKINGFINISHED);
                            barrierOvertaking = false;
                            ++barrierOvertakingCnt;
                        }
                    } else {
                        barrierDistance = 0;
                        aroundBarrier = false;
                    }
                } else {
                    if(leader_car) {
                        if(barrierDistance < 5500) {
                            if(!placeholder) {
                                placeholder = true;
                                barrierOvertaking = true;
                                SendMessage(BARIOVERTAKING);
                            }
                            lastAlong = along;
                            along = (barrierType == LeftBarrier ? AlongRightRoad : AlongLeftRoad);
                        } else if(barrierDistance < 6500) {
                            placeholder = false;
                            lastAlong = along;
                            along = (barrierType == LeftBarrier ? AlongLeftBorder : AlongRightBorder);
                        } else if(barrierDistance < 11000) {
                            lastAlong = along;
                            along = (barrierType == LeftBarrier ? AlongLeftRoad : AlongRightRoad);
                        } else if(barrierDistance < 16000) {
                            if(!placeholder) {
                                beingOvertaken = true;
                                placeholder = true;
                            } else {
                                lastAlong = along;
                                along = (barrierType == LeftBarrier ? AlongLeftRoad : AlongRightRoad);
                            }
                        } else if(barrierDistance < 19500) {
                            lastAlong = along;
                            along = AsUsual;
                        } else {
                            barrierDistance = 0;
                            aroundBarrier = false;
                            placeholder = false;
                            barrierOvertaking = false;
                            SendMessage(OVERTAKINGFINISHED);
                            ++barrierOvertakingCnt;
                        }
                    } else if(barrierOvertaking) {
                        if(barrierDistance < 15000) {
                            lastAlong = along;
                            along = (barrierType == LeftBarrier ? AlongRightRoad : AlongLeftRoad);
                        } else if(!placeholder) {
                            placeholder = true;
                            SendMessage(OVERTAKINGFINISHED);
                            beingOvertaken = true;
                        } else {
                            barrierDistance = 0;
                            aroundBarrier = false;
                            barrierOvertaking = false;
                            placeholder = false;
                            ++barrierOvertakingCnt;
                        }
                    } else {
                        barrierDistance = 0;
                        aroundBarrier = false;
                    }
                }
            } else {
                if(leader_car || barrierOvertaking) {
                    if(barrierDistance < 5500) {
                        lastAlong = along;
                        along = (barrierType == LeftBarrier ? AlongRightRoad : AlongLeftRoad);
                    } else {
                        if(leader_car) {
                            SendMessage(OVERTAKINGFINISHED);
                        }
                        barrierDistance = 0;
                        aroundBarrier = false;
                        lastAlong = along;
                        along = AsUsual;
                    }
                } else {
                    barrierDistance = 0;
                    aroundBarrier = false;
                }
            }
        } else if(barrierDistance < 5500) {
            lastAlong = along;
            along = (barrierType == LeftBarrier ? AlongRightRoad : AlongLeftRoad);
        } else {
            barrierDistance = 0;
            aroundBarrier = false;
            lastAlong = along;
            along = AsUsual;
        }
    } else if(afterCrossRoad) {
        if(leader_car) {
            if(afterCrossRoadDistance < 5000) {
                return DummyRightBarrier;
            } else {
                afterCrossRoad = false;
                afterCrossRoadDistance = 0;
                leader_car = !leader_car;
                beingOvertaken = true;
                ++crossRoadOvertakingCnt;
            }
        } else {
            if(afterCrossRoadDistance > 12000) {
                afterCrossRoad = false;
                afterCrossRoadDistance = 0;
                leader_car = !leader_car;
                SendMessage(OVERTAKINGFINISHED);
                ++crossRoadOvertakingCnt;
            }
        }
    } else if(onRamp) {
        if(double_car && rampOvertakingEnabled && rampOvertakingCnt < rampOvertakingCntMax) {
            if(leader_car) {
                if(rampDistance > 18000) {
                    beingOvertaken = true;
                    leader_car = !leader_car;
                    onRamp = false;
                    rampDistance = 0;
                    ++rampOvertakingCnt;
                    placeholder = false;
                } else if(rampDistance > 11000) {
                    lastAlong = along;
                    along = AlongLeftRoad;
                } else {
                    if(!placeholder) {
                        SendMessage(RAMPOVERTAKING);
                        placeholder = true;
                    }
                }
            } else {
                if(rampDistance > 18000) {
                    leader_car = !leader_car;
                    onRamp = false;
                    rampDistance = 0;
                    SendMessage(OVERTAKINGFINISHED);
                    ++rampOvertakingCnt;
                } else if(rampDistance > 15000) {
                    lastAlong = along;
                    along = AsUsual;
                } else {
                    lastAlong = along;
                    along = AlongRightRoad;
                }
            }
        } else {
            if(rampDistance > 16000) {
                onRamp = false;
                rampDistance = 0;
            }
        }
    } else if(rampOvertaking) {
        int16_t _cnt = 0;
        for(int16_t row = 30; row < 38; ++row) {
            if((resultSet.rightBorder[row] == resultSet.leftBorder[row])
                || (resultSet.rightBorder[row] - resultSet.leftBorder[row] < 60
                && resultSet.leftBorder[row] < 70
                && resultSet.rightBorder[row] > 155)) {
                ++_cnt;
            }
        }
        if(_cnt > 3) {
            rampOvertaking = false;
            onRamp = true;
        }
    } else if(straightLineOvertakingEnabled
        && straightLineOvertakingCnt < straightLineOvertakingCntMax
        && leader_car
        && straightLine
        && !inStraightLine
        && !resultSet.leftBorderNotFoundCnt
        && !resultSet.rightBorderNotFoundCnt) {
        inStraightLine = true;
    } else if(inStraightLine) {
        if(leader_car) {
            if(straightLineDistance < 800) {
                lastAlong = along;
                along = AlongLeftBorder;
            } else if(straightLineDistance < 5400) {
                lastAlong = along;
                along = AlongLeftRoad;
            } else {
                beingOvertaken = true;
                leader_car = !leader_car;
                inStraightLine = false;
                straightLineDistance = 0;
                ++straightLineOvertakingCnt;
                SendMessage(STRAOVERTAKING);
            }
        } else {
            if(straightLineDistance > 18000) {
                leader_car = !leader_car;
                inStraightLine = false;
                straightLineDistance = 0;
                SendMessage(OVERTAKINGFINISHED);
                ++straightLineOvertakingCnt;
            } else if(straightLineDistance > 12000) {
                lastAlong = along;
                along = AsUsual;
            } else {
                lastAlong = along;
                along = AlongRightRoad;
            }
        }
    } else if(straightLineOvertaking) {
        int16_t _cnt = 0;
        for(int16_t row = 30; row < 38; ++row) {
            if(resultSet.rightBorder[row] - resultSet.leftBorder[row] < 95) {
                ++_cnt;
            }
        }
        if(_cnt > 3) {
            straightLineOvertaking = false;
            inStraightLine = true;
        }
    }
    int16_t temp;
    
    return enabled && !inRing && !preRingEnd && !ringEndDelay && !inCrossRoad && IsRing() ? Ring
        : enabled && !inRing &&!preRingEnd &&!ringEndDelay && !inCrossRoad && IsHugeRing() ? HugeRing
//        : enabled && ((double_car && leader_car) || (!double_car)) && !inRing && !ringEndDelay && IsCrossRoad() ? CrossRoad
        : enabled && (!double_car || leader_car) && !inRing && !ringEndDelay && !inCrossRoad && !onRamp && IsRamp() ? Ramp
        : enabled && (!double_car || leader_car || barrierOvertaking) && !beingOvertaken && !onRamp && !inRing && !ringEndDelay && !inCrossRoad && (temp = WhichBarrier()) != Unknown ? temp
        : !final && start_line && startLineEnabled && (!double_car || leader_car) && !onRamp && IsStartLine(startLinePresight) ? Startline
        : Unknown;
}

bool IsOutOfRoad() {
    int16_t cnt = 0;
    int16_t line = 0;
    for(int16_t row = 0; row < 10; ++row) {
        for(int16_t col = 60; col < IMG_COL - 60; ++col) {
            if(IsBlack(row, col)) {
                ++cnt;
            }
        }
        if(cnt >= 85) {
            if(++line >= 4) {
                return true;
            }
        }
        cnt = 0;
    }
    return false;
}

bool IsStartLine(int16_t row) {
    int16_t toggleCnt = 0;
    int16_t patternRowCnt = 0;
    #define ABCDEFG 22
    if(IsBlack(row + ABCDEFG, resultSet.middleLine[row + ABCDEFG])) {
        return false;
    }
    width__ = resultSet.rightBorder[row + ABCDEFG] - resultSet.leftBorder[row + ABCDEFG] + 21;
    for(int16_t i = row; i < row + ABCDEFG - 2; ++i) {
        toggleCnt = 0;
        for(int16_t j = IMG_COL / 2 - width__ / 2; j < IMG_COL / 2 + width__ / 2; ++j) {
            if(TstImgBufAsBitMap(i, j) != TstImgBufAsBitMap(i, j+1)) {
                if(++toggleCnt >= 10) {
                    ++patternRowCnt;
                    break;
                }
            }
        }
        if(patternRowCnt >= 3) {
            break;
        }
        if(toggleCnt < 10) {
            patternRowCnt = 0;
        }
    }
    if(patternRowCnt >= 3) {
        int16_t middleAreaCnt = 0;
        for(int16_t i = 0; i < row - 4; ++i) {
            if(IsBlack(i, resultSet.middleLine[i])) {
                return false;
            }
            if(InRange(resultSet.middleLine[i], IMG_COL / 2 - 30, IMG_COL / 2 + 30)) {
                ++middleAreaCnt;
            }
        }
        for(int16_t i = row + 10 + 4; i < IMG_ROW; ++i) {
            if(IsBlack(i, resultSet.middleLine[i])) {
                return false;
            }
            if(InRange(resultSet.middleLine[i], IMG_COL / 2 - 30, IMG_COL / 2 + 30)) {
                ++middleAreaCnt;
            }
        }
        return middleAreaCnt > 15;
    } else {
        return false;
    }
}

bool IsStraightLine() {
    int16_t middleAreaOffset = 0;
    if(InRange(resultSet.middleLine[1], 95, 130)
        && !resultSet.leftBorderNotFoundCnt
        && !resultSet.rightBorderNotFoundCnt) {
        for(int16_t i = 2; i < IMG_ROW; ++i) {
            if(IsBlack(i, resultSet.middleLine[i])) {
                return false;
            }
            middleAreaOffset +=
                Abs(resultSet.middleLine[i] - resultSet.middleLine[1]);
        }
        return InRange(middleAreaOffset, 0, 90);
    } else {
        return false;
    }
}

void StartlineAction() {
    if(double_car) {
        if(leader_car) {
            final = true;
            SendMessage(FINAL);
        }
    } else {
        final = true;
        finalPursueingFinished = true;
    }
}

bool IsRamp() {
    int16_t middleAreaCnt = 0;
    for(int16_t i = 0; i < IMG_ROW; ++i) {
        if(IsBlack(i, resultSet.middleLine[i])) {
            return false;
        }
        if(InRange(resultSet.middleLine[i], IMG_COL / 2 - 30, IMG_COL / 2 + 30)) {
            ++middleAreaCnt;
        }
    }
    return middleAreaCnt > 38
        && resultSet.rightBorder[48] - resultSet.leftBorder[48] > 60
        && resultSet.leftBorder[48] > 50 && resultSet.rightBorder[48] < 180
        && !resultSet.leftBorderNotFoundCnt
        && !resultSet.rightBorderNotFoundCnt
        && InRange(resultSet.rightBorder[42] - resultSet.leftBorder[42], 70, 120)
        && InRange(resultSet.rightBorder[41] - resultSet.leftBorder[41], 70, 120);
}
