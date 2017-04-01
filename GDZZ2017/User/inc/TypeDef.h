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

typedef uint8_t byte;

#define NUMBER_OF_MODES 16

typedef void (*mode_type)(void);
typedef mode_type mode_type_array[NUMBER_OF_MODES];

#define NUMBER_OF_IMG_PROC IMG_ROW_INTV

typedef void (*img_proc_type)(void);
typedef img_proc_type img_proc_type_array[NUMBER_OF_IMG_PROC];

typedef struct _img_proc_result_set_type {
    int16_t leftBorder[IMG_ROW];
    int16_t rightBorder[IMG_ROW];
    int16_t middleLine[IMG_ROW];
    bool foundLeftBorder[IMG_ROW];
    bool foundRightBorder[IMG_ROW];
    float leftSlope[IMG_ROW];
    float leftZero[IMG_ROW];
    float rightSlope[IMG_ROW];
    float rightZero[IMG_ROW];
    float middleSlope[IMG_ROW];
    float middleZero[IMG_ROW];
    int16_t leftTrend[IMG_ROW];
    int16_t rightTrend[IMG_ROW];
    int16_t middleLineMaxRow;
    int16_t middleLineMinRow;
    uint16_t imgProcFlag;
}
img_proc_result_set_type;

#define ULTRA_SONIC_TIME_QUEUE_LENGTH 10

typedef struct _ultra_sonic_time_queue_type {
    uint32_t valueBuf[ULTRA_SONIC_TIME_QUEUE_LENGTH];
    uint8_t cursor;
}
ultra_sonic_time_queue_type;

#endif
