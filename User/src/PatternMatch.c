#include "PatternMatch.h"
#include "ImgProc.h"
#include "DoubleCar.h"
#include "MainProc.h"
#include "Action.h"
#include "Gyro.h"
#include "gpio.h"
#include "SpeedControl.h"
#include "Motor.h"



int16_t startLineWidth;
static bool placeholder;

int16_t GetRoadType() {
    if(ringEndDelay) {
        if(ringDistance < 3200) {
            return RingEnd;
        } else {
            ringEndDelay = false;
            ringDistance = 0;
        }
    } else if(inRing) {
        if(ringDistance > 42000) {
            inRing = false;
            ringDistance = 0;
        } else if(ringDistance > 4400 && IsRingEnd()) {
            inRing = false;
            ringDistance = 0;
            ringEndDelay = true;
            return RingEnd;
        } else if(ringDistance < 2000) {
            return Ring;
        }
    } else if(aroundBarrier) {
        if(barrierOvertaking) {
            if(leader_car) {
                if(barrierDistance < 7500) {
                    return barrierType;
                } else if(barrierDistance < barrierOvertakingDistanceMax) {
                    if(!placeholder) {
                        placeholder = true;
                        SendMessage(START);
                    }
                    return barrierType == LeftBarrier ? DummyRightBarrier : DummyLeftBarrier;
                } else {
                    barrierDistance = 0;
                    aroundBarrier = false;
                    leader_car = !leader_car;
                    beingOvertaken = true;
                    barrierOvertaking = false;
                }
            } else {
                if(barrierDistance < 10000) {
                    return barrierType;
                } else if(barrierDistance > 18000) {
                    barrierDistance = 0;
                    aroundBarrier = false;
                    leader_car = !leader_car;
                    SendMessage(OVERTAKINGFINISHED);
                    barrierOvertaking = false;
                }
            }
        } else {
            if(barrierDistance > 1500) {
                barrierDistance = 0;
                aroundBarrier = false;
            } else {
                return barrierType;
            }
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
        if(leader_car) {
            if(rampDistance > 16000) {
                beingOvertaken = true;
                leader_car = !leader_car;
                onRamp = false;
                rampDistance = 0;
                SendMessage(RAMPOVERTAKING);
            } else if(rampDistance > 11000) {
                along = AlongLeftBorder;
            }
        } else {
            if(rampDistance > 16000) {
                leader_car = !leader_car;
                onRamp = false;
                rampDistance = 0;
                SendMessage(OVERTAKINGFINISHED);
                along = AsUsual;
            } else {
                along = AlongRightBorder;
            }
        }
    } else if(rampOvertaking) {
        int16_t _cnt = 0;
        for(int16_t row = 30; row < 38; ++row) {
            if(resultSet.rightBorder[row] - resultSet.leftBorder[row] < 60) {
                ++_cnt;
            }
        }
        if(_cnt > 3) {
            rampOvertaking = false;
            onRamp = true;
        }
    }
    
//    int16_t curve = WhichCurve();
    
    return /*curve != Unknown ? curve
        :*/ !inRing && !ringEndDelay && !inCrossRoad && IsRing() ? Ring
        : (double_car ? leader_car : true)
            && !inRing && !ringEndDelay && IsCrossRoad() ? CrossRoad
        : enabled && double_car && leader_car
            && !inRing && !ringEndDelay && !inCrossRoad && !onRamp && IsRamp() ? Ramp
        : enabled && !inRing && !ringEndDelay && !inCrossRoad ? WhichBarrier()
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
    for(int16_t i = row; i < row + 7; ++i) {
        for(int16_t j = IMG_COL / 2 - startLineWidth / 2; j < IMG_COL / 2 + startLineWidth / 2; ++j) {
            if(TstImgBufAsBitMap(i, j) != TstImgBufAsBitMap(i, j+1)) {
                if(++toggleCnt >= 8) {
                    toggleCnt = 0;
                    ++patternRowCnt;
                }
            }
        }
    }
    if(patternRowCnt >= 3) {
        int16_t middleAreaCnt = 0;
        for(int16_t i = 0; i < row - 8; ++i) {
            if(InRange(resultSet.middleLine[i], IMG_COL / 2 - 30, IMG_COL / 2 + 30)) {
                ++middleAreaCnt;
            }
        }
        for(int16_t i = row + 7 + 8; i < IMG_ROW; ++i) {
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
    if(InRange(resultSet.middleLine[0], 100, 125)) {
        for(int16_t i = 1; i < IMG_ROW; ++i) {
            middleAreaOffset +=
                Abs(resultSet.middleLine[i] - resultSet.middleLine[0]);
        }
        return InRange(middleAreaOffset, 0, 90);
    } else {
        return false;
    }
}

bool IsRamp() {
    return IsStraightLine()
        && resultSet.rightBorder[48] - resultSet.leftBorder[48] > 70
        && resultSet.leftBorder[48] > 50 && resultSet.rightBorder[48] < 180
        && !resultSet.leftBorderNotFoundCnt
        && !resultSet.rightBorderNotFoundCnt
        && InRange(resultSet.rightBorder[42] - resultSet.leftBorder[42], 80, 120);
}
