#ifndef _DATACOMM_H
#define _DATACOMM_H

#include "root.h"

void DataCommInit(void);
void ImgTrans(uint8_t imgBuf[OV7725_H][OV7725_W / 2]);
void StateTrans(float a, float b, float c, float d);

#endif
