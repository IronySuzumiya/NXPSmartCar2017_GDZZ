#ifndef _ENCODER_H
#define _ENCODER_H

#include "root.h"

void EncoderInit(void);
void EncoderGet(int16_t* left, int16_t* right);
void EncoderClear(void);

extern bool encoder_on;

#endif
