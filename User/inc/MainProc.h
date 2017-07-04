#ifndef _MAINPROC_H
#define _MAINPROC_H

#include "root.h"

extern inline int16_t Abs(int16_t input) {
    return input >= 0 ? input : -input;
}

extern inline int16_t Min(int16_t a, int16_t b) {
    return a > b ? b : a;
}

extern inline int16_t Max(int16_t a, int16_t b) {
    return a > b ? a : b;
}

extern inline int16_t Min_f(float a, float b) {
    return a > b ? b : a;
}

extern inline int16_t Max_f(float a, float b) {
    return a > b ? a : b;
}

extern inline bool OpstSign(int16_t a, int16_t b) {
    return a * b < 0;
}

extern inline bool InRange(int16_t value, int16_t lbound, int16_t hbound) {
    return value > lbound && value < hbound;
}

extern inline float Limit(int16_t value, int16_t lbound, int16_t hbound) {
    return value > hbound ? hbound : value < lbound ? lbound : value;
}

extern inline float Limit_f(float value, float lbound, float hbound) {
    return value > hbound ? hbound : value < lbound ? lbound : value;
}

void MainInit(void);

extern bool enabled;
extern int16_t waitForOvertakingCnt;
extern int16_t overtakingCnt;

#endif
