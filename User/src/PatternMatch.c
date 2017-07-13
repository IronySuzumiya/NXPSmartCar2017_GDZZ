#include "PatternMatch.h"
#include "ImgProc.h"
#include "DoubleCar.h"
#include "MainProc.h"

int32_t crossRoadDistanceLeaderMax;
int32_t crossRoadDistanceFollowerMax;
int16_t startLineWidth;

int16_t GetRoadType() {
    if(ringEndDelay) {
        if(ringDistance < 3200) {
            return RingEnd;
        } else {
            ringDistance = 0;
            ringEndDelay = false;
            ringInterval = true;
        }
    } else if(ringInterval) {
        if(ringDistance > 4000) {
            ringDistance = 0;
            ringInterval = false;
        }
    } else if(inRing) {
        if(ringDistance > 42000) {
            ringDistance = 0;
            inRing = inBigRing = false;
        } else if(ringDistance > 4400 && IsRingEnd()) {
            ringDistance = 0;
            ringEndDelay = true;
            inRing = inBigRing = false;
            return RingEnd;
        } else if(ringDistance < 2000) {
            return Ring;
        }
    } else if(inCrossRoad) {
        if(/*(!double_car && */crossRoadDistance > crossRoadDistanceLeaderMax/*)
            || (leader_car && crossRoadDistance > crossRoadDistanceLeaderMax)
            || (!leader_car && crossRoadDistance > crossRoadDistanceFollowerMax)*/) {
            inCrossRoad = false;
            crossRoadDistance = 0;
        }
    } else if(aroundBarrier) {
        if(barrierDistance > 1200) {
            barrierDistance = 0;
            aroundBarrier = false;
        } else {
            return barrierType;
        }
    }
    
//    int16_t curve = WhichCurve();
    
    return /*curve != Unknown ? curve
        :*/ !inRing && !ringEndDelay && !ringInterval && !inCrossRoad && IsRing() ? Ring
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
