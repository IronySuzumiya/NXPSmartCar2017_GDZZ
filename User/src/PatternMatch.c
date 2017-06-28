#include "PatternMatch.h"
#include "ImgUtility.h"
#include "ImgProc.h"
#include "Motor.h"
#include "stdio.h"
#include "uart.h"
#include "gpio.h"

int32_t ringDistance;
int32_t crossRoadDistance;
bool inRing;
bool ringEndDelay;
bool inCrossRoad;

int16_t barrierType;
int32_t barrierDistance;
bool aroundBarrier;

static bool IsRing(void);
static bool IsRingEndFromLeft(void);
static bool IsRingEndFromRight(void);
static bool IsCrossRoad(void);
static int16_t WhichCurve(void);
static int16_t WhichBarrier(void);

static int16_t black_pt_row;
static int16_t last_not_found_border_row;

int16_t GetRoadType() {
    if(ringEndDelay) {
        if(ringDistance < 10000) {
            return RingEnd;
        } else {
            ringDistance = 0;
            ringEndDelay = false;
        }
    } else if(inRing) {
        if(ringDistance > 90000) {
            ringDistance = 0;
            inRing = false;
        } else if(ringDistance > 14000 && IsRingEnd()) {
            ringDistance = 0;
            ringEndDelay = true;
            inRing = false;
            return RingEnd;
        } else if(ringDistance < 10000) {
            return Ring;
        }
    } else if(inCrossRoad) {
        if(crossRoadDistance > 80000) {
            crossRoadDistance = 0;
            inCrossRoad = false;
        }
    } else if(aroundBarrier) {
        if(barrierDistance > 5000) {
            barrierDistance = 0;
            aroundBarrier = false;
        } else {
            return barrierType;
        }
    }
    
    int16_t curve = WhichCurve();
    
    return curve != Unknown ? curve
        : !inRing && !inCrossRoad && IsRing() ? Ring
        : !inRing && !ringEndDelay && IsCrossRoad() ? CrossRoad
        : !inRing && !ringEndDelay && !inCrossRoad ? WhichBarrier()
        : Unknown;
}

bool IsRing() {
//    float acc = 0;
//    float _cursor = resultSet.middleLine[4];
//    int16_t cursor;
//    for(int16_t i = 1; i <= 8; ++i) {
//        acc += resultSet.middleLine[i] - resultSet.middleLine[i-1];
//    }
//    acc /= 8;
//    int16_t row;
//    int16_t _cnt = 0;
//    for(row = 5; row < 40; ++row) {
//        _cursor += acc + 0.5;
//        cursor = _cursor;
//        if(IsBlack(row, cursor)) {
//            break;
//        }
//        if(resultSet.rightBorder[row] - resultSet.leftBorder[row] > 200) {
//            ++_cnt;
//        }
//    }
    int16_t cursor = 0;
    for(int16_t i = 0; i < 5; ++i) {
        cursor += resultSet.middleLine[i];
    }
    cursor /= 5;
    int16_t row;
    int16_t _cnt = 0;
    for(row = 5; row < 40; ++row) {
        if(IsBlack(row, cursor)) {
            break;
        }
        if(resultSet.rightBorder[row] - resultSet.leftBorder[row] > 200) {
            ++_cnt;
        }
    }
    if(_cnt < 10 || row == 40) {
        return false;
    }
    bool hasRightOffset = false;
    for(int16_t col = cursor; col < Min(cursor + 15, IMG_COL); ++col) {
        if(IsBlack(row - 1, col)) {
            hasRightOffset = true;
            cursor = col;
            --row;
            if(row == 0) {
                return false;
            }
        }
    }
    if(!hasRightOffset) {
        for(int16_t col = cursor; col > Max(cursor - 15, -1); --col) {
            if(IsBlack(row - 1, col)) {
                cursor = col;
                --row;
                if(row == 0) {
                    return false;
                }
            }
        }
    }
    int16_t left, right;
    int16_t cnt[4] = { 0 };
    int16_t minWidth, maxWidth;
    int16_t leftMost, rightMost;
    
    left = right = cursor;
    for(; IsBlack(row, left) && left > 0; --left) { }
    for(; IsBlack(row, right) && right < IMG_COL - 1; ++right) { }
    if(IsBlack(row, left) && IsBlack(row, right)) {
        return false;
    }
    leftMost = left;
    rightMost = right;
    minWidth = maxWidth = right - left;
    ++row;
    
    for(; row < IMG_ROW; ++row) {
        left = right = cursor;
        if(IsWhite(row, cursor)) {
            break;
        }
        for(; IsBlack(row, left) && left > 0; --left) { }
        for(; IsBlack(row, right) && right < IMG_COL - 1; ++right) { }
        if(IsBlack(row, left) && IsBlack(row, right)) {
            return false;
        }
        if(leftMost > left) {
            leftMost = left;
            ++cnt[0];
        } else if(leftMost < left) {
            ++cnt[1];
        }
        if(rightMost < right) {
            rightMost = right;
            ++cnt[2];
        } else if(rightMost > right) {
            ++cnt[3];
        }
        maxWidth = Max(right - left, maxWidth);
        cursor = (left + right) / 2;
    }
    
    return inRing = leftMost < 80 && cnt[0] > 3 && cnt[1] > 3 && cnt[0] < 7
        /*&& Abs(cnt[0] - cnt[1]) < 4*/ && maxWidth > 75 && maxWidth - minWidth > 50;
}

bool IsRingEndFromLeft() {
    int16_t row;
    int16_t cnt = 0;
    for(row = 20; row < IMG_COL - 1 && IsWhite(row, resultSet.middleLine[row]); ++row) {
        if(resultSet.leftSlope[row] * resultSet.leftSlope[row - 5] < 0) {
            ++cnt;
        }
    }
    return cnt > 1;
}

bool IsRingEndFromRight() {
    int16_t row;
    int16_t cnt = 0;
    for(row = 20; row < IMG_COL - 1 && IsWhite(row, resultSet.middleLine[row]); ++row) {
        if(resultSet.rightSlope[row] * resultSet.rightSlope[row - 5] < 0) {
            ++cnt;
        }
    }
    return cnt > 1;
}

bool IsCrossRoad() {
    int16_t cnt = 0;
    for(int16_t i = 5; i < 30; ++i) {
        if(!resultSet.foundLeftBorder[i] && !resultSet.foundRightBorder[i]) {
            ++cnt;
        }
    }
    for(int16_t i = 30; i < 45; ++i) {
        if(IsBlack(i, resultSet.middleLine[i])
            || resultSet.middleLine[i] > IMG_COL - 60
            || resultSet.middleLine[i] < 60) {
            return false;
        }
    }
    return inCrossRoad = cnt > 6;
}

int16_t WhichCurve() {
    int16_t row;
    bool leftCurve = false;
    bool rightCurve = false;
    int16_t cnt = 0;
    for (row = 5; row < IMG_ROW && IsWhite(row, resultSet.middleLine[row]); ++row) { }
    if(row < IMG_ROW && !InRange(resultSet.middleLine[row], IMG_COL / 2 - 50, IMG_COL / 2 + 50)) {
        black_pt_row = row;
        for(; row >= 0; --row) {
            if(!resultSet.foundLeftBorder[row]) {
                leftCurve = true;
                break;
            } else if(!resultSet.foundRightBorder[row]) {
                rightCurve = true;
                break;
            }
        }
        if(leftCurve) {
            for(; row >= 0; --row) {
                if(!resultSet.foundLeftBorder[row] && resultSet.foundRightBorder[row]) {
                    ++cnt;
                    if(cnt > 5) {
                        return LeftCurve;
                    }
                }
            }
        } else if(rightCurve) {
            for(; row >= 0; --row) {
                if(!resultSet.foundRightBorder[row] && resultSet.foundLeftBorder[row]) {
                    ++cnt;
                    if(cnt > 5) {
                        return RightCurve;
                    }
                }
            }
        }
    }
    return Unknown;
}

int16_t WhichBarrier() {
    int16_t inRow;
    int16_t outRow;
    int16_t row;
    int16_t _barrierType;
    for(row = 10; row < 45 && Abs(resultSet.middleLine[row] - resultSet.middleLine[row - 2]) <= 16; ++row) { }
    if(!InRange(resultSet.middleLine[row - 2], IMG_COL / 2 - 20, IMG_COL / 2 + 20)) {
        return Unknown;
    }
    
    _barrierType = resultSet.middleLine[row] - resultSet.middleLine[row - 2] > 0 ? LeftBarrier : RightBarrier;
    
    inRow = row;
    row += 2;
    for(; row < IMG_ROW && Abs(resultSet.middleLine[row] - resultSet.middleLine[row - 2]) <= 10; ++row) { }
    if((resultSet.middleLine[row] - resultSet.middleLine[row - 2] > 0 && _barrierType == LeftBarrier)
        || (resultSet.middleLine[row] - resultSet.middleLine[row - 2] < 0 && _barrierType == RightBarrier)) {
        return Unknown;
    }
    outRow = row;
    if(outRow - inRow > 5 && row < IMG_ROW && resultSet.middleLine[row] > IMG_COL / 2 - 20
        && resultSet.middleLine[row] < IMG_COL / 2 + 20) {
        aroundBarrier = true;
        return barrierType = _barrierType;
    } else {
        return Unknown;
    }
}

void RingCompensateGoLeft() {
    for(int16_t i = pre_sight - 3; i < pre_sight + 3; ++i) {
        resultSet.middleLine[i] = 0;
    }
}

void RingCompensateGoRight() {
    for(int16_t i = pre_sight - 3; i < pre_sight + 3; ++i) {
        resultSet.middleLine[i] = IMG_COL - 1;
    }
}

void RingEndCompensateFromLeft() {
//    for(int16_t i = pre_sight - 3; i < pre_sight + 3; ++i) {
//        resultSet.middleLine[i] = 0;
//    }
}

void RingEndCompensateFromRight() {
//    for(int16_t i = pre_sight - 3; i < pre_sight + 3; ++i) {
//        resultSet.middleLine[i] = IMG_COL - 1;
//    }
}

void LeftCurveCompensate() {
    for (int row_ = IMG_ROW - 1; row_ > black_pt_row; --row_)
    {
        resultSet.middleLine[row_] = 0;
    }
    for (int cnt = 0; cnt < 12; ++cnt)
    {
        resultSet.middleLine[black_pt_row - cnt] = cnt * resultSet.middleLine[black_pt_row - 12] / 12;
    }
}

void RightCurveCompensate() {
    for (int row_ = IMG_ROW - 1; row_ > black_pt_row; --row_)
    {
        resultSet.middleLine[row_] = IMG_COL - 1;
    }
    for (int cnt = 0; cnt < 12; ++cnt)
    {
        resultSet.middleLine[black_pt_row - cnt] = IMG_COL - 1
            - cnt * (IMG_COL - 1 - resultSet.middleLine[black_pt_row - 12]) / 12;
    }
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

bool StartLineJudge(int16_t row) {
    int16_t toggleCnt = 0;
    int16_t patternRowCnt = 0;
    for(int16_t i = row; i >= row - 6; --i) {
        for(int16_t j = IMG_COL / 2; j >= 0; --j) {
            if(TstImgBufAsBitMap(i, j) != TstImgBufAsBitMap(i, j+1)) {
                if(toggleCnt > 14) {
                    toggleCnt = 0;
                    ++patternRowCnt;
                    if(patternRowCnt > 4) {
                        return true;
                    } else {
                        break;
                    }
                } else {
                    ++toggleCnt;
                }
            }
        }
    }
    return false;
}

bool StraightLineJudge(void) {
    int16_t middleAreaCnt = 0;
    for(int16_t i = 0; i < IMG_ROW; ++i) {
        if(InRange(resultSet.middleLine[i], IMG_COL / 2 - 10, IMG_COL / 2 + 10)) {
            middleAreaCnt++;
        }
    }
    return middleAreaCnt > 38;
}
