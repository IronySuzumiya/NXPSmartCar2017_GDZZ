#ifndef _MAINPROC_H
#define _MAINPROC_H

#include "root.h"

struct _facility {
    struct _nvic {
        void (*init)(void);
    } nvic;
    struct _buzzle {
        void (*init)(void);
        void (*on)(void);
        void (*off)(void);
    } buzzle;
    struct _timer {
        void (*init)(struct _timer *self);
        void (*change_stat)(bool option);
        uint32_t period;
        void (*callback)(void);
    } timer;
};

struct _ui {
    struct _oled {
        void (*init)(struct _oled *self);
        const char *title;
    } oled;
};

extern inline int16_t Abs(int16_t input) {
    return input >= 0 ? input : -input;
}

extern inline int16_t Min(int16_t a, int16_t b) {
    return a > b ? b : a;
}

extern inline int16_t Max(int16_t a, int16_t b) {
    return a > b ? a : b;
}

extern inline float Min_f(float a, float b) {
    return a > b ? b : a;
}

extern inline float Max_f(float a, float b) {
    return a > b ? a : b;
}

extern inline bool OpstSign(int16_t a, int16_t b) {
    return a * b < 0;
}

extern inline bool InRange(int16_t value, int16_t lbound, int16_t hbound) {
    return value > lbound && value < hbound;
}

extern inline bool InRange_f(float value, float lbound, float hbound) {
    return value > lbound && value < hbound;
}

extern inline int16_t Limit(int16_t value, int16_t lbound, int16_t hbound) {
    return value > hbound ? hbound : value < lbound ? lbound : value;
}

extern inline float Limit_f(float value, float lbound, float hbound) {
    return value > hbound ? hbound : value < lbound ? lbound : value;
}

void OLEDPrintf(uint8_t x, uint8_t y, char *str, ...);
void OLEDClrRow(uint8_t row);
void MainInit(void);

extern bool enabled;
extern int32_t startLineEnableDistance;

#endif
