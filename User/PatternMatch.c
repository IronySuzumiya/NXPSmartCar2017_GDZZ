#include "PatternMatch.h"
#include "ImgUtility.h"
#include "ImgProc.h"
#include "Motor.h"
#include "stdio.h"
#include "uart.h"
#include "gpio.h"

int32_t ringDistance;
bool inRing;
bool ringEndDelay;

int16_t barrierType;
int32_t barrierDistance;
bool aroundBarrier;

static bool IsRing(void);
static bool IsRingEndFromLeft(void);
static bool IsRingEndFromRight(void);
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
        if(ringDistance > 200000L) {
            ringDistance = 0;
            inRing = false;
        } else if(ringDistance > 25000 && IsRingEnd()) {
            ringDistance = 0;
            ringEndDelay = true;
            inRing = false;
            return RingEnd;
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
        : IsRing() ? Ring
        : (!inRing && !ringEndDelay ? WhichBarrier() : Unknown);
}

bool IsRing() {
    int16_t cursor = 0;
    for(int16_t i = 0; i < 5; ++i) {
        cursor += resultSet.middleLine[i];
    }
    cursor /= 5;
    int16_t row;
    int16_t _cnt = 0;
    for(row = 5; row < IMG_ROW; ++row) {
        if(IsBlack(row, cursor)) {
            break;
        }
        if(resultSet.rightBorder[row] - resultSet.leftBorder[row] > 150) {
            ++_cnt;
        }
    }
    if(_cnt < 8 || row == IMG_ROW) {
        return false;
    }
    bool hasRightOffset = false;
    for(int16_t col = cursor; col < Min(cursor + 7, IMG_COL); ++col) {
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
        for(int16_t col = cursor; col > Max(cursor - 7, -1); --col) {
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
    int16_t cnt = 0;
    int16_t minWidth, maxWidth;

    left = right = cursor;
    for(; IsBlack(row, left) && left > Max(cursor - 10, 0); --left) { }
    for(; IsBlack(row, right) && right < Min(cursor + 10, IMG_COL - 1); ++right) { }
    minWidth = maxWidth = right - left;
    ++row;

    for(; row < IMG_ROW; ++row) {
        left = right = cursor;
        for(; IsBlack(row, left) && left > Max(cursor - 10, 0); --left) { }
        for(; IsBlack(row, right) && right < Min(cursor + 10, IMG_COL - 1); ++right) { }
        int16_t width = right - left;
        if(width <= maxWidth) {
            if(++cnt > 3) {
                break;
            }
        } else {
            cnt = 0;
            maxWidth = width;
        }
        cursor = (left + right) / 2;
    }

    return maxWidth > 80 && maxWidth - minWidth > 50;
}

bool IsRingEndFromLeft() {
    if(resultSet.rightBorderNotFoundCnt < 15) {
        return false;
    }
    int16_t row;
    for(row = 0; row < IMG_ROW && resultSet.foundRightBorder[row]; ++row) { }
    for(; row < IMG_ROW && !resultSet.foundRightBorder[row]; ++row) { }
    last_not_found_border_row = row;
    
    for(row = 20; row < 40; ++row) {
        if(OpstSign(resultSet.leftTrend[row], resultSet.leftTrend[row - 5])) {
            break;
        }
    }
    if(row >= 40) {
        return false;
    }
    
    int16_t cnt = 0;
    for(row = IMG_ROW - 1; row >= 35; --row) {
        if(IsWhite(row, resultSet.middleLine[row])) {
            ++cnt;
        }
    }
    return cnt > 12;
}

bool IsRingEndFromRight() {
    if(resultSet.leftBorderNotFoundCnt < 15) {
        return false;
    }
    int16_t row;
    for(row = 0; row < IMG_ROW && resultSet.foundLeftBorder[row]; ++row) { }
    for(; row < IMG_ROW && !resultSet.foundLeftBorder[row]; ++row) { }
    last_not_found_border_row = row;
    
    for(row = 20; row < 40; ++row) {
        if(OpstSign(resultSet.rightTrend[row], resultSet.rightTrend[row - 5])) {
            break;
        }
    }
    if(row >= 40) {
        return false;
    }
    
    int16_t cnt = 0;
    for(row = IMG_ROW - 1; row >= 35; --row) {
        if(IsWhite(row, resultSet.middleLine[row])) {
            ++cnt;
        }
    }
    return cnt > 12;
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
    for(int16_t i = 0; i < IMG_ROW; ++i) {
        resultSet.middleLine[i] = 0;
    }
}

void RingCompensateGoRight() {
    for(int16_t i = 0; i < IMG_ROW; ++i) {
        resultSet.middleLine[i] = IMG_COL - 1;
    }
}

void RingEndCompensateFromLeft() {
    int16_t row;
    int16_t col;
    for(row = last_not_found_border_row; row < IMG_ROW; ++row) {
        if(resultSet.rightBorder[row] < IMG_COL - 40) {
            col = resultSet.rightBorder[row];
            break;
        }
    }
    if(row < IMG_ROW) {
        --row;
        for(; row >= 0; --row) {
            resultSet.rightBorder[row] = col;
        }
        MiddleLineUpdateAll();
    }
}

void RingEndCompensateFromRight() {
    int16_t row;
    int16_t col;
    for(row = last_not_found_border_row; row < IMG_ROW; ++row) {
        if(resultSet.leftBorder[row] > 40) {
            col = resultSet.leftBorder[row];
            break;
        }
    }
    if(row < IMG_ROW) {
        --row;
        for(; row >= 0; --row) {
            resultSet.leftBorder[row] = col;
        }
        MiddleLineUpdateAll();
    }
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
