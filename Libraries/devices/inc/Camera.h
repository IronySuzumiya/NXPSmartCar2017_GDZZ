#ifndef _CAMERA_H
#define _CAMERA_H

#include "root.h"

struct _camera {
    void (*init)(struct _camera *self);
    void (*change_stat)(bool option);
    void (*callback_href)(uint32_t pinxArray);
    void (*callback_vsyn)(uint32_t pinxArray);
    byte img_buffer[IMG_ROW][IMG_COL];
};

extern struct _camera camera;

#endif
