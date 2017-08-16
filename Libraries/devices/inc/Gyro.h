#ifndef _GYRO_H
#define _GYRO_H

#include "root.h"

struct _gyro {
    void (*init)(struct _gyro *self);
    int32_t (*get_value)(void);
    bool work;
};

extern struct _gyro gyro;

#endif
