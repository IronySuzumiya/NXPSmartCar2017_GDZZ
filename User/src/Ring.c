#include "Ring.h"
#include "MainProc.h"
#include "ImgProc.h"

int32_t ringDistance;
bool inRing;
bool ringEndDelay;
bool ringInterval;

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
    
    return leftMost < 80 && cnt[0] > 3 && cnt[1] > 3 && cnt[0] < 7 && maxWidth > 75 && maxWidth - minWidth > 50;
}

bool IsRingEndFromLeft() {
    double a = 9.0 / (resultSet.leftBorder[47] - resultSet.leftBorder[38]);
    double b = 9.0 * resultSet.leftBorder[38] / (resultSet.leftBorder[38] - resultSet.leftBorder[47]);
    int16_t r = 0;
    for(int16_t row = 38; row < 48; ++row) {
        r += Abs(resultSet.leftBorder[row] - (row - 38 - b) / a);
    }
    return r < 15;
}

bool IsRingEndFromRight() {
    double a = 9.0 / (resultSet.rightBorder[47] - resultSet.rightBorder[38]);
    double b = 9.0 * resultSet.rightBorder[38] / (resultSet.rightBorder[38] - resultSet.rightBorder[47]);
    int16_t r = 0;
    for(int16_t row = 38; row < 48; ++row) {
        r += Abs(resultSet.rightBorder[row] - (row - 38 - b) / a);
    }
    return r < 15;
}

void RingActionGoLeft() {
    for(int16_t i = pre_sight - 3; i < pre_sight + 3; ++i) {
        resultSet.middleLine[i] = 0;
    }
}

void RingActionGoRight() {
    for(int16_t i = pre_sight - 3; i < pre_sight + 3; ++i) {
        resultSet.middleLine[i] = IMG_COL - 1;
    }
}

void RingEndActionFromLeft() {
    
}

void RingEndActionFromRight() {
    
}
