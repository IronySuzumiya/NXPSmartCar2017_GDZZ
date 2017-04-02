#include "BorderSearchRelative.h"
#include "ImgProc.h"
#include "gpio.h"

static bool LeftBorderSearchInRange(int16_t row, int16_t startIndex);
static bool RightBorderSearchInRange(int16_t row, int16_t startIndex);

bool LeftBorderSearch(int16_t row) {
    if(row == 0) { //first line
        return LeftBorderSearchInRange(row, IMG_COL / 2);
    } else {
        return LeftBorderSearchInRange(row, resultSet.middleLine[row - 1]);
    }
}

bool LeftBorderSearchInRange(int16_t row, int16_t startIndex) {
    for(int16_t j = startIndex; j >= 0; --j) {
        if(TstImgBufAsBitMap(row, j) && !TstImgBufAsBitMap(row, j+1)) {
            resultSet.leftBorder[row] = j;
            return true;
        }
    }
    resultSet.leftBorder[row] = 0;
    return false;
}

bool RightBorderSearch(int16_t row) {
    if(row == 0) { //first line
        return RightBorderSearchInRange(row, IMG_COL / 2);
    } else {
        if(resultSet.rightBorder[row - 1] - resultSet.leftBorder[row - 1] <= wide_road_size) {
            return RightBorderSearchInRange(row, resultSet.middleLine[row - 1]);
        } else {
            return RightBorderSearchInRange(row, resultSet.middleLine[row - 1] - img_border_scan_compensation);
        }
    }
}

bool RightBorderSearchInRange(int16_t row, int16_t startIndex) {
    for(int16_t j = startIndex; j < IMG_COL; ++j) {
        if(!TstImgBufAsBitMap(row, j-1) && TstImgBufAsBitMap(row, j)) {
            resultSet.rightBorder[row] = j;
            return true;
        }
    }
    resultSet.rightBorder[row] = IMG_COL - 1;
    return false;
}

void MiddleLineUpdate(int16_t row) {
    resultSet.middleLine[row] = (resultSet.leftBorder[row] + resultSet.rightBorder[row]) / 2;
}

void MiddleLineRangeUpdate(int16_t row) {
    if(row > 4 && row < mini_s_visual_field) {
        if(resultSet.middleLine[row] < resultSet.middleLine[resultSet.middleLineMinRow]) {
            resultSet.middleLineMinRow = row;
        } else if(resultSet.middleLine[row] > resultSet.middleLine[resultSet.middleLineMaxRow]) {
            resultSet.middleLineMaxRow = row;
        }
    }
}
