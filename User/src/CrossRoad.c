#include "CrossRoad.h"
#include "ImgProc.h"

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
