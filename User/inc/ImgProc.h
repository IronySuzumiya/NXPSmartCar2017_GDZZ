#ifndef _IMGPROC_H
#define _IMGPROC_H

#include "root.h"

void ImgProcInit(void);

#ifdef USE_BMP
void SetImgBufAsBitMap(int16_t row, int16_t col);
void ClrImgBufAsBitMap(int16_t row, int16_t col);
bool TstImgBufAsBitMap(int16_t row, int16_t col);
extern byte imgBuf[IMG_ROW][1 + IMG_COL / 8];
#else
#define TstImgBufAsBitMap(row, col) (imgBuf[row][col])
extern byte imgBuf[IMG_ROW][IMG_COL];
#endif

#define IsWhite(row, col) (!TstImgBufAsBitMap(row, col))
#define IsBlack(row, col) (TstImgBufAsBitMap(row, col))

extern bool direction_control_on;
extern int16_t pre_sight_default;
extern int16_t pre_sight;
extern img_proc_struct resultSet;
extern bool waitForOvertaking;
extern bool overtaking;
extern bool aroundOvertaking;
extern bool firstOvertakingFinished;
extern int32_t startDistance;
extern bool final;
extern bool finalOvertakingFinished;
extern int32_t finalDistance;
extern int32_t wholeDistance;
extern bool startLineEnabled;
extern bool goAlongLeft;

#endif
