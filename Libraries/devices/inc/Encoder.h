#ifndef _ENCODER_H
#define _ENCODER_H

#include "root.h"

struct _encoder {
    void (*init)(struct _encoder *self);
    void (*get_value)(struct _encoder *self);
    void (*clr_value)(void);
    int16_t left_value;
    int16_t right_value;
    bool work;
};

extern struct _encoder encoder;

#endif
