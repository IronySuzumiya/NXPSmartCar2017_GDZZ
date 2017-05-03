#include "Encoder.h"
#include "ftm.h"
#include "gpio.h"
#include "lptmr.h"

bool encoder_on;

void EncoderInit() {
    FTM_QD_QuickInit(ENCODER_LEFT_MAP_FTM_QD,  kFTM_QD_NormalPolarity, kQD_CountDirectionEncoding);
    FTM_QD_ClearCount(ENCODER_LEFT_PORT_FTM_QD);
    #ifdef ALL_FTM_QD
        FTM_QD_QuickInit(ENCODER_RIGHT_MAP_FTM_QD, kFTM_QD_NormalPolarity, kQD_CountDirectionEncoding);
        FTM_QD_ClearCount(ENCODER_RIGHT_PORT_FTM_QD);
    #else
        LPTMR_PC_QuickInit(ENCODER_RIGHT_VALUE_MAP_LPTMR);
        GPIO_QuickInit(ENCODER_RIGHT_DIRECTION_PORT_LPTMR, ENCODER_RIGHT_DIRECTION_PIN_LPTMR, kGPIO_Mode_IPU);
    #endif
}

void EncoderGet(int16_t* left, int16_t* right) {
    if(encoder_on) {
        uint8_t leftDir, rightDir;
        FTM_QD_GetData(ENCODER_LEFT_PORT_FTM_QD, left, &leftDir);
        *left = -*left;
        FTM_QD_ClearCount(ENCODER_LEFT_PORT_FTM_QD);
        #ifdef ALL_FTM_QD
            FTM_QD_GetData(ENCODER_RIGHT_PORT_FTM_QD, right, &rightDir);
            FTM_QD_ClearCount(ENCODER_RIGHT_PORT_FTM_QD);
        #else
            if(ENCODER_RIGHT_DIRECTION_READ_LPTMR) {
                *right = LPTMR_PC_ReadCounter();
            } else {
                *right = -LPTMR_PC_ReadCounter();
            }
            LPTMR_ClearCounter();
        #endif
        leftDir = leftDir;
        rightDir = rightDir;
    } else {
        FTM_QD_ClearCount(ENCODER_LEFT_PORT_FTM_QD);
        #ifdef ALL_FTM_QD
            FTM_QD_ClearCount(ENCODER_RIGHT_PORT_FTM_QD);
        #else
            LPTMR_ClearCounter();
        #endif
        *left = *right = 0;
    }
}
