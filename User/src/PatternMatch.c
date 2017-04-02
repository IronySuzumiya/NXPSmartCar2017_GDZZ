#include "PatternMatch.h"
#include "ImgProc.h"
#include "stdio.h"
#include "uart.h"

//static void MiniSCompensate(int16_t* middleLine);

void CrossRoadJudge(int16_t row) {
    if(resultSet.rightBorder[row] - resultSet.leftBorder[row] > cross_road_size && row >= slope_sensitivity) {
        resultSet.leftBorder[row] = row * resultSet.leftSlope[row - 1] + resultSet.leftZero[row - 1];
        resultSet.rightBorder[row] = row * resultSet.rightSlope[row - 1] + resultSet.rightZero[row - 1];
        resultSet.imgProcFlag |= CROSS_ROAD;
        resultSet.middleLine[row] = (resultSet.leftBorder[row] + resultSet.rightBorder[row]) / 2;
    }
}

void StraightRoadJudge(int16_t* middleLine) {
    int16_t middleAreaCnt = 0;
    for(int16_t i = 0; i < IMG_ROW; ++i) {
        if(middleLine[i] > IMG_COL / 2 - straight_road_sensitivity && middleLine[i] < IMG_COL / 2 + straight_road_sensitivity) {
            middleAreaCnt++;
        }
    }
    if(middleAreaCnt > straight_road_middle_area_cnt_min) {
        resultSet.imgProcFlag |= STRAIGHT_ROAD;
    }
}

void MiniSJudge(int16_t* middleLine, int16_t minRow, int16_t maxRow) {
//    int16_t cnt = 0;
    if(middleLine[maxRow] - middleLine[minRow] < mini_s_sensitivity 
        && middleLine[maxRow] - middleLine[minRow] > 18
        && middleLine[maxRow] < IMG_COL / 2 + 30
        && middleLine[minRow] > IMG_COL / 2 - 30
        && resultSet.foundBorderCnt > 45
        && (!imgBuf[IMG_ROW - 1][IMG_COL / 2] || !imgBuf[IMG_ROW - 2][IMG_COL / 2])) {
        
        resultSet.imgProcFlag |= MINI_S;
        pre_sight = 47;
        //MiniSCompensate(resultSet.middleLine);
    } else {
        pre_sight = 20;
    }
}

//void MiniSCompensate(int16_t* middleLine) {
//    int16_t avgMiddleLine = 0;
//    for(int16_t i = 0; i < IMG_ROW; ++i) {
//        avgMiddleLine += middleLine[i];
//    }
//    avgMiddleLine /= IMG_ROW;
//    avgMiddleLine = (avgMiddleLine + middleLine[IMG_ROW - 1] + middleLine[IMG_ROW - 2]) / 3;
//    for(int16_t i = pre_sight - slope_sensitivity; i < pre_sight + slope_sensitivity + 2; ++i) {
//        middleLine[i] = avgMiddleLine;
//    }
//}

void StartLineJudge(int16_t row) {
    int16_t toggleCnt = 0;
    int16_t patternRowCnt = 0;
    for(int16_t i = row; i >= row - startline_sensitivity; --i) {
        for(int16_t j = IMG_COL / 2; j >= 0; --j) {
            if(TstImgBufAsBitMap(i, j) != TstImgBufAsBitMap(i, j+1)) {
                if(toggleCnt > startline_black_tape_num * 2) {
                    toggleCnt = 0;
                    ++patternRowCnt;
                    if(patternRowCnt > startline_sensitivity / 2 + 1) {
                        resultSet.imgProcFlag |= START_LINE;
                        return;
                    } else {
                        break;
                    }
                } else {
                    ++toggleCnt;
                }
            }
        }
    }
}

void CurveJudge(int16_t row) {
    int16_t missBorderCnt = 0;
    uint16_t turnDirection = 0;
    int16_t begin, end;
    while(resultSet.foundLeftBorder[row] && resultSet.foundRightBorder[row]) { ++row; }
    begin = row;
    for(; row < IMG_ROW; ++row) {
        if(turnDirection != TURN_RIGHT && !resultSet.foundLeftBorder[row] && resultSet.foundRightBorder[row]) {
            ++missBorderCnt;
            turnDirection = TURN_LEFT;
        } else if(turnDirection != TURN_LEFT && resultSet.foundLeftBorder[row] && !resultSet.foundRightBorder[row]) {
            ++missBorderCnt;
            turnDirection = TURN_RIGHT;
        } else if(missBorderCnt > curve_sensitivity) {
            end = row;
            break;
        } else {
            missBorderCnt = 0;
        }
    }
    if(!end) {
        return;
    }
    int16_t diff;
    int16_t border;
//    static int16_t cnt = 0;
//    static char buf[150];
    if(turnDirection == TURN_LEFT) {
        border = 0;
        resultSet.imgProcFlag |= TURN_LEFT;
    } else {
        border = IMG_COL - 1;
        resultSet.imgProcFlag |= TURN_RIGHT;
    }
    diff = (border - resultSet.middleLine[begin - 1]) / (end - begin);
//    if(cnt > 50) {
//        sprintf(buf, "beginat=%d, endat=%d, beginpos=%d\r\n", begin - 1, end, resultSet.middleLine[begin - 1]);
//        UART_printf(DATACOMM_IMG_TRANS_CHL, buf);
//    }
    for(int16_t i = begin; i < end; ++i) {
        resultSet.middleLine[i] = resultSet.middleLine[begin - 1] + (i - (begin - 1)) * diff;
//        if(cnt > 50) {
//            sprintf(buf, "pos%d=%d\r\n", i, resultSet.middleLine[i]);
//            UART_printf(DATACOMM_IMG_TRANS_CHL, buf);
//        }
    }
    for(int16_t i = end; i < IMG_ROW; ++i) {
        resultSet.middleLine[i] = border;
//        if(cnt > 50) {
//            sprintf(buf, "pos%d=%d\r\n", i, resultSet.middleLine[i]);
//            UART_printf(DATACOMM_IMG_TRANS_CHL, buf);
//        }
    }
//    if(cnt > 50) {
//        UART_printf(DATACOMM_IMG_TRANS_CHL, "\r\n");
//        cnt = 0;
//    }
//    ++cnt;
}
