#ifndef _STEER_ACTUATOR_H
#define _STEER_ACTUATOR_H

#include "root.h"

struct _steer {
    void (*init)(struct _steer *self);
    void (*change_duty)(int16_t out);
    int16_t middle_duty;
    bool work;
};

extern struct _steer steer;

#endif
