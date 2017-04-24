#include "PatternMatch.h"
#include "ImgUtility.h"
#include "ImgProc.h"
#include "Motor.h"
#include "Utility.h"
#include "stdio.h"
#include "uart.h"
#include "gpio.h"

static bool IsRing(void);
static bool IsRingEnd(void);
static int16_t IsCurve(void);
static bool IsCrossRoad(void);

static bool definitelyInRing;
static bool inRing;
static int16_t black_pt_row;

int16_t GetRoadType() {
    static int16_t _cnt = 0;
    if(inRing && !definitelyInRing) {
        ++_cnt;
        if(_cnt > 30) {
            _cnt = 0;
            definitelyInRing = true;
        }
    }
    if(definitelyInRing && IsRingEnd()) {
        inRing = false;
        definitelyInRing = false;
        return RingEnd;
    }
    int16_t curve = IsCurve();
    if(curve == Unknown) {
        int16_t cnt = 0;
        int16_t row;
        for(row = 5; row < IMG_ROW; ++row) {
            if((resultSet.rightBorder[row] - resultSet.leftBorder[row])
                - (resultSet.rightBorder[row - 5] - resultSet.leftBorder[row - 5]) > 6
                && resultSet.rightBorder[row] - resultSet.leftBorder[row] > 140) {
                ++cnt;
            }
            if(cnt > 3) {
                break;
            }
        }
        if(row == IMG_ROW) {
            BUZZLE_OFF;
            return Unknown;
        }
        bool leftCnt = false, rightCnt = false;
        int16_t _row = row;
        for(; row >= _row - 10; --row) {
            if((resultSet.leftTrend[row] & 0x8000) ^ (resultSet.leftTrend[row - 2] & 0x8000)) {
                leftCnt = true;
            }
            if((resultSet.rightTrend[row] & 0x8000) ^ (resultSet.rightTrend[row - 2] & 0x8000)) {
                rightCnt = true;
            }
            if(leftCnt && rightCnt) {
                BUZZLE_ON;
                return IsRing() ? Ring : IsCrossRoad() ? CrossRoad : Unknown;
            }
        }
        BUZZLE_OFF;
        return Unknown;
    } else {
        BUZZLE_OFF;
        return curve;
    }
}

bool IsRing() {
    int16_t blackBlockRowsCnt = 0;
    int16_t col;
    int16_t whiteCol;
    int16_t width;
    for (int16_t row = IMG_ROW - 1; row >= 30; --row)
    {
        if(IsWhite(row, IMG_COL / 2))
        {
            continue;
        }
        for(col = IMG_COL / 2 - 1; IsBlack(row, col) && col >= 0; --col) { }
        if(col == -1 || col >= IMG_COL / 2 - 5)
        {
            continue;
        }
        for(whiteCol = col; IsWhite(row, whiteCol) && whiteCol >= 0; --whiteCol) { }
        width = col - whiteCol;
        for (col = IMG_COL / 2 + 1; IsBlack(row, col) && col < IMG_COL; ++col) { }
        if(col >= IMG_COL / 2 + 5 && col < IMG_COL)
        {
            for(whiteCol = col; IsWhite(row, whiteCol) && whiteCol < IMG_COL; ++whiteCol) { }
            if(Abs((whiteCol - col) - width) < 30) {
                ++blackBlockRowsCnt;
            }
        }
    }
    if(blackBlockRowsCnt > 6) {
        inRing = true;
        return true;
    }
    return false;
}

bool IsRingEnd() {
    if(resultSet.rightBorderNotFoundCnt > 10) {
        int16_t row;
        int16_t cnt;
        for(row = IMG_ROW - 1; row >= 0 && resultSet.foundRightBorder[row]; --row) { }
        for(; row >= 0 && !((resultSet.leftTrend[row] & 0x8000) ^ (resultSet.leftTrend[row - 2] & 0x8000)); --row) {
            if((resultSet.rightBorder[row + 5] - resultSet.leftBorder[row + 5])
                - (resultSet.rightBorder[row] - resultSet.leftBorder[row]) > 6
                && resultSet.rightBorder[row + 5] - resultSet.leftBorder[row + 5] > 150) {
                ++cnt;
            }
            if(cnt > 3) {
                return true;
            }
        }
    }
    return false;
}

int16_t IsCurve() {
    int16_t row;
    bool leftCurve = false;
    bool rightCurve = false;
    int16_t cnt = 0;
    for (row = 5; row < IMG_ROW && IsWhite(row, resultSet.middleLine[row]); ++row) { }
    if(row < IMG_ROW && ((resultSet.middleLine[row] < IMG_COL / 2 - 22) || (resultSet.middleLine[row] > IMG_COL / 2 + 22))) {
        black_pt_row = row;
        for(; row >= 0; --row) {
            if(!leftCurve && !rightCurve) {
                if(!resultSet.foundLeftBorder[row]) {
                    leftCurve = true;
                } else if(!resultSet.foundRightBorder[row]) {
                    rightCurve = true;
                }
            } else if(leftCurve) {
                if(!resultSet.foundLeftBorder[row] && resultSet.foundRightBorder[row]) {
                    ++cnt;
                }
                if(cnt > 5) {
                    return LeftCurve;
                }
            } else if(rightCurve) {
                if(!resultSet.foundRightBorder[row] && resultSet.foundLeftBorder[row]) {
                    ++cnt;
                }
                if(cnt > 5) {
                    return RightCurve;
                }
            }
        }
    }
    return Unknown;
}

bool IsCrossRoad() {
    return resultSet.leftBorderNotFoundCnt > 2 && resultSet.rightBorderNotFoundCnt > 2
        && resultSet.leftBorderNotFoundCnt + resultSet.rightBorderNotFoundCnt > 10;
}

void RingCompensateGoLeft() {
    int row;
    for (row = IMG_ROW - 1;
        row >= 30 && IsWhite(row, IMG_COL / 2); --row) { }
    for (; row >= 10 && IsBlack(row, IMG_COL / 2); --row) { }
    int col;
    for (col = IMG_COL / 2; col >= 0 && IsBlack(row + 1, col); --col) { }
    for(int i = row; i > 0; --i)
    {
        resultSet.rightBorder[i] = col + (row + 1 - i) * (resultSet.rightBorder[0] - col) / row;
        MiddleLineUpdate(i);
    }
    int16_t borderSearchStart = col / 2;
    for (int i = row; i < IMG_ROW; ++i)
    {
        LeftBorderSearchFrom(i, borderSearchStart);
        RightBorderSearchFrom(i, borderSearchStart);
        MiddleLineUpdate(i);
        borderSearchStart = resultSet.middleLine[i];
    }
}

void RingCompensateGoRight() {
    int row;
    for (row = IMG_ROW - 1;
        row >= 30 && IsWhite(row, IMG_COL / 2); --row) { }
    for (; row >= 10 && IsBlack(row, IMG_COL / 2); --row) { }
    int col;
    for (col = IMG_COL / 2; col < IMG_COL && IsBlack(row + 1, col); ++col) { }
    for (int i = row; i > 0; --i)
    {
        resultSet.leftBorder[i] = col - (row + 1 - i) * (col - resultSet.leftBorder[0]) / row;
        MiddleLineUpdate(i);
    }
    int16_t borderSearchStart = col + (IMG_COL - col) / 2;
    for (int i = row; i < IMG_ROW; ++i)
    {
        LeftBorderSearchFrom(i, borderSearchStart);
        RightBorderSearchFrom(i, borderSearchStart);
        MiddleLineUpdate(i);
        borderSearchStart = resultSet.middleLine[i];
    }
}

void RingEndCompensateFromLeft() {
    motor_on = false;
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
        resultSet.middleLine[black_pt_row - cnt] = IMG_COL - 1 - cnt * (IMG_COL - 1 - resultSet.middleLine[black_pt_row - 12]) / 12;
    }
}

void CrossRoadCompensate() {
    int leftCompensateStart = IMG_ROW;
    int rightCompensateStart = IMG_ROW;
    int leftCompensateEnd = IMG_ROW;
    int rightCompensateEnd = IMG_ROW;

    {
        int row = 6;
        while (row < IMG_ROW && resultSet.leftBorder[row] != 0
            && Abs(resultSet.leftSlope[row] - resultSet.leftSlope[row - 1]) < 3 && resultSet.leftSlope[row] * resultSet.leftSlope[row - 1] >= 0) { ++row; }
        leftCompensateStart = row;
    }

    {
        int row = 6;
        while (row < IMG_ROW && resultSet.rightBorder[row] != IMG_COL - 1
            && Abs(resultSet.rightSlope[row] - resultSet.rightSlope[row - 1]) < 3 && resultSet.leftSlope[row] * resultSet.leftSlope[row - 1] >= 0) { ++row; }
        rightCompensateStart = row;
    }

    for (int row = leftCompensateStart; row < leftCompensateEnd; ++row)
    {
        resultSet.leftBorder[row] = row * resultSet.leftSlope[leftCompensateStart - 5] + resultSet.leftZero[leftCompensateStart - 5];
        if(IsBlack(row, resultSet.leftBorder[row]))
        {
            leftCompensateEnd = row;
            break;
        }
    }

    for (int row = rightCompensateStart; row < rightCompensateEnd; ++row)
    {
        resultSet.rightBorder[row] = row * resultSet.rightSlope[rightCompensateStart - 5] + resultSet.rightZero[rightCompensateStart - 5];
        if (IsBlack(row, resultSet.rightBorder[row]))
        {
            rightCompensateEnd = row;
            break;
        }
    }

    int borderSearchStart;

    if (leftCompensateEnd < rightCompensateEnd)
    {
        borderSearchStart = (resultSet.leftBorder[leftCompensateEnd - 1] + resultSet.rightBorder[leftCompensateEnd - 1]) / 2;
        for (int row = leftCompensateEnd; row < rightCompensateEnd; ++row)
        {
            LeftBorderSearchFrom(row, borderSearchStart);
            //if (Math.Abs((resultSet.rightBorder[row] + resultSet.leftBorder[row]) - (resultSet.rightBorder[row - 1] + resultSet.leftBorder[row - 1])) < 10)
            //{
                borderSearchStart = (resultSet.rightBorder[row] + resultSet.leftBorder[row]) / 2;
            //}
        }
    }
    else if (leftCompensateEnd > rightCompensateEnd)
    {
        borderSearchStart = (resultSet.leftBorder[rightCompensateEnd - 1] + resultSet.rightBorder[rightCompensateEnd - 1]) / 2;
        for (int row = rightCompensateEnd; row < leftCompensateEnd; ++row)
        {
            RightBorderSearchFrom(row, borderSearchStart);
            //if (Math.Abs((resultSet.rightBorder[row] + resultSet.leftBorder[row]) - (resultSet.rightBorder[row - 1] + resultSet.leftBorder[row - 1])) < 10)
            //{
                borderSearchStart = (resultSet.rightBorder[row] + resultSet.leftBorder[row]) / 2;
            //}
        }
    }
    
    int compensateEnd = Max(leftCompensateEnd, rightCompensateEnd);

    borderSearchStart = (resultSet.leftBorder[compensateEnd - 1] + resultSet.rightBorder[compensateEnd - 1]) / 2;
    for (int row = compensateEnd; row < IMG_ROW; ++row)
    {
        LeftBorderSearchFrom(row, borderSearchStart);
        RightBorderSearchFrom(row, borderSearchStart);
        if (Abs((resultSet.rightBorder[row] + resultSet.leftBorder[row]) - (resultSet.rightBorder[row - 1] + resultSet.leftBorder[row - 1])) < 10)
        {
            borderSearchStart = (resultSet.rightBorder[row] + resultSet.leftBorder[row]) / 2;
        }
    }
    
    MiddleLineUpdateAll();
}

bool StartLineJudge(int16_t row) {
    int16_t toggleCnt = 0;
    int16_t patternRowCnt = 0;
    for(int16_t i = row; i >= row - 6; --i) {
        for(int16_t j = IMG_COL / 2; j >= 0; --j) {
            if(TstImgBufAsBitMap(i, j) != TstImgBufAsBitMap(i, j+1)) {
                if(toggleCnt > 7 * 2) {
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
        if(resultSet.middleLine[i] > (IMG_COL / 2 - 10) && resultSet.middleLine[i] < (IMG_COL / 2 + 10)) {
            middleAreaCnt++;
        }
    }
    if(middleAreaCnt > 38) {
        return true;
    } else {
        return false;
    }
}
