#ifndef _MOTOR_H
#define _MOTOR_H

#include "root.h"

struct _motor {
    void (*init)(struct _motor *self);
    void (*change_duty)(int16_t left, int16_t right);
    bool work;
};

extern struct _motor motor;

#endif
