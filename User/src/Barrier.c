#include "Barrier.h"
#include "ImgProc.h"
#include "MainProc.h"

int16_t barrierType;
int32_t barrierDistance;
bool aroundBarrier;

int16_t WhichBarrier() {
    int16_t inRow;
    int16_t outRow;
    int16_t row;
    int16_t _barrierType;
    for(row = 10; row < 35 && Abs(resultSet.middleLine[row] - resultSet.middleLine[row - 2]) <= 16; ++row) { }
    if(!InRange(resultSet.middleLine[row - 2], IMG_COL / 2 - 30, IMG_COL / 2 + 30)) {
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
        int16_t cnt = 0;
        int16_t rowCnt = 0;
        if(_barrierType == LeftBarrier) {
            for(int16_t row = inRow; row < outRow; ++row) {
                for(int16_t col = resultSet.middleLine[row]; col >= 40; --col) {
                    if(TstImgBufAsBitMap(row, col) != TstImgBufAsBitMap(row, col + 1)) {
                        if(++cnt > 2) {
                            cnt = 0;
                            ++rowCnt;
                            break;
                        }
                    }
                }
            }
        } else {
            for(int16_t row = inRow; row < outRow; ++row) {
                for(int16_t col = resultSet.middleLine[row]; col < IMG_COL - 40; ++col) {
                    if(TstImgBufAsBitMap(row, col - 1) != TstImgBufAsBitMap(row, col)) {
                        if(++cnt > 2) {
                            cnt = 0;
                            ++rowCnt;
                            break;
                        }
                    }
                }
            }
        }
        if(rowCnt > 0.6 * (outRow - inRow)) {
            aroundBarrier = true;
            return barrierType = _barrierType;
        } else {
            return Unknown;
        }
    } else {
        return Unknown;
    }
}
