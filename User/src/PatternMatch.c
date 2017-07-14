#include "PatternMatch.h"
#include "ImgProc.h"
#include "DoubleCar.h"
#include "MainProc.h"
#include "Action.h"

int16_t startLineWidth;

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
            if(!leader_car && barrierDistance > 1200) {
                barrierDistance = 0;
                aroundBarrier = false;
            } else if(leader_car && barrierDistance > 12000) {
                barrierDistance = 0;
                aroundBarrier = false;
                beingOvertaken = true;
            } else if(leader_car && barrierDistance > 7500) {
                return barrierType == LeftBarrier ? DummyRightBarrier : DummyLeftBarrier;
            } else {
                return barrierType;
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
        if(afterCrossRoadDistance < 5000) {
            return DummyRightBarrier;
        } else {
            afterCrossRoad = false;
            afterCrossRoadDistance = 0;
            beingOvertaken = true;
        }
    }
    
//    int16_t curve = WhichCurve();
    
    return /*curve != Unknown ? curve
        :*/ !inRing && !ringEndDelay && !inCrossRoad && IsRing() ? Ring
        : (double_car ? leader_car : true)
            && !inRing && !ringEndDelay && IsCrossRoad() ? CrossRoad
        : !inRing && !ringEndDelay && !inCrossRoad ? WhichBarrier()
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
        for(int16_t i = 0; i < row; ++i) {
            if(InRange(resultSet.middleLine[i], IMG_COL / 2 - 10, IMG_COL / 2 + 10)) {
                ++middleAreaCnt;
            }
        }
        for(int16_t i = row + 7; i < IMG_ROW; ++i) {
            if(InRange(resultSet.middleLine[i], IMG_COL / 2 - 10, IMG_COL / 2 + 10)) {
                ++middleAreaCnt;
            }
        }
        return middleAreaCnt > 30;
    } else {
        return false;
    }
}

bool IsStraightLine(void) {
    int16_t middleAreaCnt = 0;
    for(int16_t i = 0; i < IMG_ROW; ++i) {
        if(InRange(resultSet.middleLine[i], IMG_COL / 2 - 10, IMG_COL / 2 + 10)) {
            ++middleAreaCnt;
        }
    }
    return middleAreaCnt > 38;
}
