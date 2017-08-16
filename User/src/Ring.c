#include "Ring.h"
#include "MainProc.h"
#include "ImgProc.h"

#pragma diag_suppress 1293

int32_t ringDistance;
bool inRing;
bool ringEndDelay;

bool IsRing() {
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
        if(resultSet.rightBorder[row] - resultSet.leftBorder[row] > 170) {
            ++_cnt;
        }
    }
    if(_cnt < 8 || row == 40) {
        return false;
    }
    bool hasRightOffset = false;
    for(int16_t col = cursor; col < Min(cursor + 20, IMG_COL); ++col) {
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
        for(int16_t col = cursor; col > Max(cursor - 20, -1); --col) {
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
#if CAR_NO == 1
    return leftMost < 85 && cnt[0] > 3 && cnt[1] > 3 && cnt[0] < 7
        && maxWidth > 65 && maxWidth - minWidth > 35
        && InRange(resultSet.middleLine[5], 112 - 20, 112 + 20)
        && InRange(resultSet.middleLine[8], 112 - 20, 112 + 20);
#elif CAR_NO == 2
    return leftMost < 85 && cnt[0] > 3 && cnt[1] > 3 && cnt[0] < 7
        && maxWidth > 65 && maxWidth - minWidth > 35
        && InRange(resultSet.middleLine[5], 112 - 20, 112 + 20)
        && InRange(resultSet.middleLine[8], 112 - 20, 112 + 20);
#endif
}

bool IsRingEndFromLeft() {
    int16_t cnt = 0;
    for(int16_t row = 30; row < 40; ++row) {
        if(IsBlack(row, resultSet.middleLine[row])) {
            break;
        }
        if(resultSet.middleLine[row] < resultSet.middleLine[row - 2]) {
            ++cnt;
        }
    }
    return cnt >= 6;
}

bool IsRingEndFromRight() {
    int16_t cnt = 0;
    for(int16_t row = 30; row < 40; ++row) {
        if(IsBlack(row, resultSet.middleLine[row])) {
            break;
        }
        if(resultSet.middleLine[row] > resultSet.middleLine[row - 2]) {
            ++cnt;
        }
    }
    return cnt >= 6;
}

void RingActionGoLeft() {
    for(int16_t i = pre_sight - 5; i < pre_sight + 5; ++i) {
        resultSet.middleLine[i] = 0;
    }
}

void RingActionGoRight() {
    for(int16_t i = pre_sight - 5; i < pre_sight + 5; ++i) {
        resultSet.middleLine[i] = IMG_COL - 1;
    }
}
