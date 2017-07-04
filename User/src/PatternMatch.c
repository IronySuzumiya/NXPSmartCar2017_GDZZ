#include "PatternMatch.h"
#include "ImgProc.h"
#include "DoubleCar.h"
#include "MainProc.h"

int16_t GetRoadType() {
    if(ringEndDelay) {
        if(ringDistance < 10000) {
            return RingEnd;
        } else {
            ringDistance = 0;
            ringEndDelay = false;
            ringInterval = true;
        }
    } else if(ringInterval) {
        if(ringDistance > 20000) {
            ringDistance = 0;
            ringInterval = false;
        }
    } else if(inRing) {
        if(ringDistance > 90000) {
            ringDistance = 0;
            inRing = false;
        } else if(ringDistance > 22000 && IsRingEnd()) {
            ringDistance = 0;
            ringEndDelay = true;
            inRing = false;
            return RingEnd;
        } else if(ringDistance < 10000) {
            return Ring;
        }
    } else if(inCrossRoad) {
        if((leader_car && crossRoadDistance > 135000L)
            || (!leader_car && crossRoadDistance > 150000L)) {
            inCrossRoad = false;
            crossRoadDistance = 0;
        }
    } else if(aroundBarrier) {
        if(barrierDistance > 5000) {
            barrierDistance = 0;
            aroundBarrier = false;
        } else {
            return barrierType;
        }
    }
    
//    int16_t curve = WhichCurve();
    
    return /*curve != Unknown ? curve
        : */!inRing && !ringEndDelay && !ringInterval && !inCrossRoad && IsRing() ? Ring
        :
        #ifdef DOUBLE_CAR
            leader_car && 
        #endif
            !inRing && !ringEndDelay && !inCrossRoad && IsCrossRoad() ? CrossRoad
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
            ++line;
            if(line >= 4) {
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
    for(int16_t i = row + 6; i >= row; --i) {
        for(int16_t j = IMG_COL / 2; j >= 60; --j) {
            if(TstImgBufAsBitMap(i, j) != TstImgBufAsBitMap(i, j+1)) {
                if(++toggleCnt >= 12) {
                    toggleCnt = 0;
                    if(++patternRowCnt >= 3) {
                        return true;
                    } else {
                        break;
                    }
                }
            }
        }
    }
    return false;
}

bool IsStraightLine(void) {
    int16_t middleAreaCnt = 0;
    for(int16_t i = 0; i < IMG_ROW; ++i) {
        if(InRange(resultSet.middleLine[i], IMG_COL / 2 - 10, IMG_COL / 2 + 10)) {
            middleAreaCnt++;
        }
    }
    return middleAreaCnt > 38;
}
