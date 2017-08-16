#include "Encoder.h"
#include "ftm.h"
#include "gpio.h"
#include "lptmr.h"

static void EncoderInit(struct _encoder *self);
static void EncoderGet(struct _encoder *self);
static void EncoderClear(void);

struct _encoder encoder = {
    EncoderInit,
    EncoderGet,
    EncoderClear,
};

void EncoderInit(struct _encoder *self) {
    FTM_QD_QuickInit(ENCODER_LEFT_MAP_FTM_QD,  kFTM_QD_NormalPolarity, kQD_CountDirectionEncoding);
    FTM_QD_ClearCount(ENCODER_LEFT_PORT_FTM_QD);
    LPTMR_PC_QuickInit(ENCODER_RIGHT_VALUE_MAP_LPTMR);
    GPIO_QuickInit(ENCODER_RIGHT_DIRECTION_PORT_LPTMR, ENCODER_RIGHT_DIRECTION_PIN_LPTMR, kGPIO_Mode_IPU);
    self->work = true;
}

void EncoderGet(struct _encoder *self) {
    uint8_t leftDir;
    FTM_QD_GetData(ENCODER_LEFT_PORT_FTM_QD, &self->left_value, &leftDir);
    self->left_value = -self->left_value;
    if(ENCODER_RIGHT_DIRECTION_READ_LPTMR) {
        self->right_value = LPTMR_PC_ReadCounter();
    } else {
        self->right_value = -LPTMR_PC_ReadCounter();
    }
    self->clr_value();
    // suppress warning
    leftDir = leftDir;
}

void EncoderClear() {
    FTM_QD_ClearCount(ENCODER_LEFT_PORT_FTM_QD);
    LPTMR_ClearCounter();
}
