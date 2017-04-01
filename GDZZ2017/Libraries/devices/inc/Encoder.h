#ifndef _ENCODER_H
#define _ENCODER_H

#define ENCODER

#include "root.h"

void EncoderInit(void);
void EncoderGet(int16_t* left, int16_t* right);

extern bool encoder_on;
extern bool use_ftm_qd, use_inc_pid;

#endif
