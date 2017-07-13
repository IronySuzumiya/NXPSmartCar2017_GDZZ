#include "CrossRoad.h"
#include "ImgProc.h"
#include "ImgUtility.h"

int32_t crossRoadDistance;
bool inCrossRoad;

bool IsCrossRoad() {
    int16_t cnt = 0;
    for(int16_t i = 5; i < 20; ++i) {
        if(!resultSet.foundLeftBorder[i] && !resultSet.foundRightBorder[i]) {
            ++cnt;
        }
    }
    for(int16_t i = 20; i < 45; ++i) {
        if(IsBlack(i, resultSet.middleLine[i])
            || resultSet.middleLine[i] > IMG_COL - 60
            || resultSet.middleLine[i] < 60) {
            return false;
        }
    }
    return cnt > 6;
}

void CrossRoadAction() {
    int16_t col;
    int16_t left;
    int16_t right;
    int16_t middle;
    for(col = IMG_COL / 2; col > 0 && IsWhite(IMG_ROW - 4, col); --col) { }
    for(; col < IMG_COL / 2 + 40 && IsBlack(IMG_ROW - 4, col); ++col) { }
    if(col == IMG_COL / 2 + 40) {
        --col;
        for(; col > IMG_COL / 2 - 40 && IsBlack(IMG_ROW - 4, col); --col) { }
        if(col == IMG_COL / 2 - 40) {
            return;
        } else {
            right = col;
            for(; col > 0 && IsWhite(IMG_ROW - 4, col); --col) { }
            left = col;
        }
    } else {
        left = col;
        for(; col < IMG_COL - 1 && IsWhite(IMG_ROW - 4, col); ++col) { }
        right = col;
    }
    middle = (left + right) / 2;
//    resultSet.leftBorder[IMG_ROW - 4] = left;
//    resultSet.rightBorder[IMG_ROW - 4] = right;
//    int16_t startIndex = resultSet.middleLine[IMG_ROW - 4] = (left + right) / 2;
//    for(int16_t row = IMG_ROW - 5; row > IMG_ROW - 10; --row) {
//        LeftBorderSearchFrom(row, startIndex);
//        RightBorderSearchFrom(row, startIndex);
//        MiddleLineUpdate(row);
//        startIndex = resultSet.middleLine[row];
//    }
    for(int16_t row = pre_sight - 3; row < pre_sight + 3; ++row) {
        resultSet.middleLine[row] = (resultSet.middleLine[5] + middle) / 2;
    }
}
