#include "BorderSearchRelative.h"
#include "ImgProc.h"
#include "gpio.h"

static bool LeftBorderSearchInRange(int16_t row, int16_t startIndex);
static bool RightBorderSearchInRange(int16_t row, int16_t startIndex);

bool LeftBorderSearchFrom(int16_t row, int16_t startIndex) {
    return LeftBorderSearchInRange(row, startIndex + 10);
}

bool LeftBorderSearchInRange(int16_t row, int16_t startIndex) {
    for(int16_t j = startIndex; j >= 0; --j) {
        if(IsBlack(row, j) && IsWhite(row, j+1)) {
            resultSet.leftBorder[row] = j;
            return true;
        }
    }
    resultSet.leftBorder[row] = 0;
    ++resultSet.leftBorderNotFoundCnt;
    return false;
}

bool RightBorderSearchFrom(int16_t row, int16_t startIndex) {
    return RightBorderSearchInRange(row, startIndex - 10);
}

bool RightBorderSearchInRange(int16_t row, int16_t startIndex) {
    for(int16_t j = startIndex; j < IMG_COL; ++j) {
        if(IsWhite(row, j-1) && IsBlack(row, j)) {
            resultSet.rightBorder[row] = j;
            return true;
        }
    }
    resultSet.rightBorder[row] = IMG_COL - 1;
    ++resultSet.rightBorderNotFoundCnt;
    return false;
}

void MiddleLineUpdate(int16_t row) {
    resultSet.middleLine[row] = (resultSet.leftBorder[row] + resultSet.rightBorder[row]) / 2;
}
