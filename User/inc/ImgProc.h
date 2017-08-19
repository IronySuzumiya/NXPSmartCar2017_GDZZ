#ifndef _IMGPROC_H
#define _IMGPROC_H

#include "root.h"

void ImgProcInit(void);

#ifdef USE_BMP

extern inline void SetImgBufAsBitMap(int16_t row, int16_t col) {
    imgBuf[row][col >> SHIFT] |= (1 << (col & MASK));
}

extern inline void ClrImgBufAsBitMap(int16_t row, int16_t col) {
    imgBuf[row][col >> SHIFT] &= ~(1 << (col & MASK));
}

extern inline bool TstImgBufAsBitMap(int16_t row, int16_t col) {
    return imgBuf[row][col >> SHIFT] & (1 << (col & MASK));
}

extern byte imgBuf[IMG_ROW][1 + IMG_COL / 8];

#else

#define TstImgBufAsBitMap(row, col) (imgBuf[row][col])

extern byte imgBuf[IMG_ROW][IMG_COL];

#endif

#define IsWhite(row, col) (!TstImgBufAsBitMap(row, col))
#define IsBlack(row, col) (TstImgBufAsBitMap(row, col))

extern img_proc_struct resultSet;
extern int16_t pre_sight;
extern int16_t startLinePresight;
extern int16_t lastAlong;
extern int16_t along;
extern bool straightLine;

void ImgProcHREF(uint32_t pinxArray);
void ImgProcVSYN(uint32_t pinxArray);

#endif
