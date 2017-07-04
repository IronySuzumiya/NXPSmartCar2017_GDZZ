#include "Ring.h"
#include "MainProc.h"
#include "ImgProc.h"

int32_t ringDistance;
bool inRing;
bool ringEndDelay;
bool ringInterval;

bool IsRing() {
    #if 0
    float acc = 0;
    float _cursor = resultSet.middleLine[4];
    int16_t cursor;
    for(int16_t i = 1; i <= 8; ++i) {
        acc += resultSet.middleLine[i] - resultSet.middleLine[i-1];
    }
    acc /= 8;
    int16_t row;
    int16_t _cnt = 0;
    for(row = 5; row < 40; ++row) {
        _cursor += acc + 0.5;
        cursor = _cursor;
        if(IsBlack(row, cursor)) {
            break;
        }
        if(resultSet.rightBorder[row] - resultSet.leftBorder[row] > 200) {
            ++_cnt;
        }
    }
    #endif
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
    
    return
//        #ifdef NO1
            leftMost < 80 && cnt[0] > 3 && cnt[1] > 3 && cnt[0] < 7
//        #else
//            rightMost >= IMG_COL - 80 && cnt[2] > 3 && cnt[3] > 3 && cnt[2] < 7
//        #endif
        /*&& Abs(cnt[0] - cnt[1]) < 4 */&& maxWidth > 75 && maxWidth - minWidth > 50;
}

bool IsRingEndFromLeft() {
    #if 0
    int16_t leftRow, rightRow, leftCol, rightCol;
    int16_t col = IMG_COL / 2;
    
    for(; col < IMG_COL / 2 + 20 && IsWhite(IMG_ROW - 1, col); ++col);
    if(col == IMG_COL / 2 + 20) {
        col = IMG_COL / 2;
        for(; col > IMG_COL / 2 - 20 && IsWhite(IMG_ROW - 1, col); --col);
        if(col == IMG_COL / 2 - 20) {
            return false;
        }
    }
    
    leftRow = rightRow = IMG_ROW - 1;
    leftCol = rightCol = col;
    for(; leftCol > 0 && IsBlack(leftRow, leftCol); --leftCol) { }
    for(; leftRow > 0 && IsBlack(leftRow, leftCol); --leftRow) { }
    for(; rightCol < IMG_COL - 1 && IsBlack(rightRow, rightCol); ++rightCol) { }
    for(; rightRow > 0 && IsBlack(rightRow, rightCol); --rightRow) { }
    
    if(leftRow < 20 || rightRow < 20) {
        return false;
    }
    
    int16_t leftArray[10] = { leftCol }, rightArray[10] = { rightCol };
    for(int16_t i = leftRow - 1; i > leftRow - 10; --i) {
        for(; leftCol < IMG_COL && IsWhite(i, leftCol); ++leftCol) { }
        leftArray[leftRow - i] = leftCol - 1;
    }
//    for(int16_t i = rightRow - 1; i > rightRow - 10; --i) {
//        for(; rightCol >= 0 && IsWhite(i, rightCol); --rightCol) { }
//        rightArray[rightRow - i] = rightCol + 1;
//    }
    double a = 9.0 / (leftArray[9] - leftArray[0]);
    double b = 9.0 * leftArray[0] / (leftArray[0] - leftArray[9]);
    int16_t r = 0;
    for(int16_t i = 0; i < 10; ++i) {
        r += Abs(leftArray[i] - (i - b) / a);
    }
    return r < 25;
    #endif
    
    double a = 9.0 / (resultSet.leftBorder[47] - resultSet.leftBorder[38]);
    double b = 9.0 * resultSet.leftBorder[38] / (resultSet.leftBorder[38] - resultSet.leftBorder[47]);
    int16_t r = 0;
    for(int16_t row = 38; row < 48; ++row) {
//        if(IsBlack(row, resultSet.middleLine[row])) {
//            return false;
//        }
        r += Abs(resultSet.leftBorder[row] - (row - 38 - b) / a);
    }
    return r < 15;
}

bool IsRingEndFromRight() {
    double a = 9.0 / (resultSet.rightBorder[47] - resultSet.rightBorder[38]);
    double b = 9.0 * resultSet.rightBorder[38] / (resultSet.rightBorder[38] - resultSet.rightBorder[47]);
    int16_t r = 0;
    for(int16_t row = 38; row < 48; ++row) {
//        if(IsBlack(row, resultSet.middleLine[row])) {
//            return false;
//        }
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
//    for(int16_t i = pre_sight - 3; i < pre_sight + 3; ++i) {
//        resultSet.middleLine[i] = 0;
//    }
}

void RingEndActionFromRight() {
//    for(int16_t i = pre_sight - 3; i < pre_sight + 3; ++i) {
//        resultSet.middleLine[i] = IMG_COL - 1;
//    }
}
