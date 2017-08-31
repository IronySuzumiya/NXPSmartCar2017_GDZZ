#ifndef __JOYSTICK_H
#define __JOYSTICK_H

#include "root.h"

#define MODIFIABLE_PARAM_NUM (sizeof(group) / sizeof(group[0]))

void JoystickInit(void);
void JoystickInt(uint32_t pinxArray);

#endif
