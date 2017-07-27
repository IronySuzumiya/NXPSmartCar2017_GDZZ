#ifndef _TYPEDEF_H
#define _TYPEDEF_H

#include "common.h"

#define SPEED_BUF_SIZE 5

typedef struct _PID {
    int16_t currentValue;
    int16_t valueBuf[SPEED_BUF_SIZE];
    uint8_t cursor;
    int16_t targetValue;
    double kp;
    double ki;
    double kd;
    int16_t lastError;
    int16_t prevError;
    int16_t sumError;
    double output;
}
PID;

typedef struct _img_proc_struct {
    int16_t leftBorder[OV7725_H];
    int16_t rightBorder[OV7725_H];
    int16_t middleLine[OV7725_H];
    bool foundLeftBorder[OV7725_H];
    bool foundRightBorder[OV7725_H];
//    int16_t leftSlope[OV7725_H];
//    int16_t leftZero[OV7725_H];
//    int16_t rightSlope[OV7725_H];
//    int16_t rightZero[OV7725_H];
//    int16_t middleSlope[OV7725_H];
//    int16_t middleZero[OV7725_H];
//    uint16_t imgProcFlag;
    int16_t leftBorderNotFoundCnt;
    int16_t rightBorderNotFoundCnt;
}
img_proc_struct;

enum _road_type {
    Unknown,
    Ring,
    RingEnd,
    LeftCurve,
    RightCurve,
    CrossRoad,
    LeftBarrier,
    RightBarrier,
    DummyLeftBarrier,
    DummyRightBarrier
};

struct ov7725_reg {
    uint8_t addr;
    uint8_t val;
};

enum ov7725_size {
    H_80_W_60,
    H_120_W_160,
    H_180_W_240,
    H_240_W_320,
};

#endif
