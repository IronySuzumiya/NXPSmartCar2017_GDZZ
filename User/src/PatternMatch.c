#include "PatternMatch.h"
#include "ImgUtility.h"
#include "ImgProc.h"
#include "stdio.h"
#include "uart.h"

static bool IsRing(void);
static bool IsCurve(void);
static bool IsCrossRoad(void);
static inline float Abs(float);
static inline float Min(float, float);
static inline float Max(float ,float);

road_type_type GetRoadType() {
    return IsRing() ? Ring :
        IsCurve() ? Curve :
        IsCrossRoad() ? CrossRoad :
        Unknown;
}

bool IsRing() {
    int blackBlockRowsCnt = 0;
    int col;
    for (int row = IMG_ROW - 1; row >= 30; --row)
    {
        if(IsWhite(row, IMG_COL / 2))
        {
            continue;
        }
        for(col = IMG_COL / 2 - 1; IsBlack(row, col) && col >= IMG_COL / 2 - 10; --col) { }
        if(col >= IMG_COL / 2 - 10)
        {
            continue;
        }
        for (col = IMG_COL / 2 + 1; IsBlack(row, col) && col <= IMG_COL / 2 + 10; ++col) { }
        if(col > IMG_COL / 2 + 10)
        {
            ++blackBlockRowsCnt;
        }
    }
    return blackBlockRowsCnt > 10;
}

bool IsCurve() {
    int blackCnt = 0;
    for (int row = IMG_ROW - 1; row >= 40; --row)
    {
        if (IsBlack(row, resultSet.middleLine[row]))
        {
            ++blackCnt;
        }
    }
    return blackCnt > 5;
}

bool IsCrossRoad() {
    return resultSet.leftBorderNotFoundCnt > 3 && resultSet.rightBorderNotFoundCnt > 3
        && resultSet.leftBorderNotFoundCnt + resultSet.rightBorderNotFoundCnt > 15;
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

void CurveCompensate() {
    int row;
    for (row = IMG_ROW - 1; row > 8; --row)
    {
        if (IsWhite(row, resultSet.middleLine[row]))
        {
            break;
        }
    }
    if (resultSet.leftBorderNotFoundCnt > 6 && resultSet.rightBorderNotFoundCnt < 2)
    {
        for (int row_ = IMG_ROW - 1; row_ > row; --row_)
        {
            resultSet.middleLine[row_] = 0;
        }
        for (int cnt = 0; cnt < 12; ++cnt)
        {
            resultSet.middleLine[row - cnt] = cnt * resultSet.middleLine[row - 12] / 12;
        }
    }
    else if (resultSet.rightBorderNotFoundCnt > 6 && resultSet.leftBorderNotFoundCnt < 2)
    {
        for (int row_ = IMG_ROW - 1; row_ > row; --row_)
        {
            resultSet.middleLine[row_] = IMG_COL - 1;
        }
        for (int cnt = 0; cnt < 12; ++cnt)
        {
            resultSet.middleLine[row - cnt] = IMG_COL - 1 - cnt * (IMG_COL - 1 - resultSet.middleLine[row - 12]) / 12;
        }
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
    
    UpdateMiddleLine();
}

inline float Abs(float input) {
    return input >= 0 ? input : -input;
}

inline float Min(float a, float b) {
    return a > b ? b : a;
}

inline float Max(float a, float b) {
    return a > b ? a : b;
}

bool StartLineJudge(int16_t row) {
    int16_t toggleCnt = 0;
    int16_t patternRowCnt = 0;
    for(int16_t i = row; i >= row - startline_sensitivity; --i) {
        for(int16_t j = IMG_COL / 2; j >= 0; --j) {
            if(TstImgBufAsBitMap(i, j) != TstImgBufAsBitMap(i, j+1)) {
                if(toggleCnt > startline_black_tape_num * 2) {
                    toggleCnt = 0;
                    ++patternRowCnt;
                    if(patternRowCnt > startline_sensitivity / 2 + 1) {
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
    for(int16_t row = 4; row < IMG_ROW - 1; ++row) {
        if(Abs(resultSet.middleSlope[row] - resultSet.middleSlope[row + 1]) > 2) {
            return false;
        }
    }
    return true;
}
