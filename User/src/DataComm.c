#include "DataComm.h"
#include "DoubleCar.h"
#include "ImgProc.h"
#include "uart.h"
#include "PatternMatch.h"

static void ImgTransInRange(img_proc_struct* resultSetPtr, int16_t startIndex, int16_t endIndex);
static void ImgTransAtPreSight(img_proc_struct* resultSetPtr);
static uint16_t CrcCheck(uint8_t *buf, uint8_t crcCnt);

void DataCommInit() {
    UART_QuickInit(DATACOMM_IMG_TRANS_MAP, DATACOMM_IMG_TRANS_BAUD);
}

void ImgTransOnlyBorderAndMiddleLine(int8_t* leftBorder, int8_t* middleLine, int8_t* rightBorder) {
    for(int16_t i = 0; i < IMG_ROW; i++) {
        UART_WriteByte(DATACOMM_IMG_TRANS_CHL, leftBorder[i]);
        UART_WriteByte(DATACOMM_IMG_TRANS_CHL, middleLine[i]);
        UART_WriteByte(DATACOMM_IMG_TRANS_CHL, rightBorder[i]);
    }
    UART_WriteByte(DATACOMM_IMG_TRANS_CHL, IMG_EOF);
}
#ifndef USE_NEW_FORMAT
void ImgTrans(img_proc_struct* resultSetPtr) {
    #ifdef HIGHLIGHT_PRESIGHT
    ImgTransInRange(resultSetPtr, IMG_ROW, pre_sight + 1);
    ImgTransAtPreSight(resultSetPtr);
    ImgTransInRange(resultSetPtr, pre_sight, 0);
    #else
    ImgTransInRange(resultSetPtr, IMG_ROW, 0);
    #endif
    UART_WriteByte(DATACOMM_IMG_TRANS_CHL, IMG_EOF);
}
#else
void ImgTrans(img_proc_struct* resultSetPtr) {
    // frame header
    UART_WriteByte(DATACOMM_IMG_TRANS_CHL, 0xee);
    UART_WriteByte(DATACOMM_IMG_TRANS_CHL, 0xee);
    UART_WriteByte(DATACOMM_IMG_TRANS_CHL, 0xee);
    UART_WriteByte(DATACOMM_IMG_TRANS_CHL, 0xee);
    UART_WriteByte(DATACOMM_IMG_TRANS_CHL, 0xee);
    #ifdef USE_BMP
        for(int16_t i = IMG_ROW - 1; i >= 0; --i) {
            for(int16_t j = 0; j < 1 + IMG_COL / 8; ++j) {
                UART_WriteByte(DATACOMM_IMG_TRANS_CHL, imgBuf[i][j]);
            }
        }
    #else
        byte tmp;
        int16_t j;
        for(int16_t i = IMG_ROW - 1; i >= 0; --i) {
            if(resultSetPtr->middleLine[i] == j
                || resultSetPtr->leftBorder[i] == j
                || resultSetPtr->rightBorder[i] == j) {
                tmp |= 0x01;
            } else {
                if (IsBlack(i, j))
                    tmp |= 0x01;
                else
                    tmp &= ~0x01;
            }
            for(j = 1; j < IMG_COL; ++j) {
                if(resultSetPtr->middleLine[i] == j
                    || resultSetPtr->leftBorder[i] == j
                    || resultSetPtr->rightBorder[i] == j) {
                    tmp |= 0x01 << (j % 8);
                } else {
                    if (IsBlack(i, j))
                        tmp |= 0x01 << (j % 8);
                    else
                        tmp &= ~(0x01 << (j % 8));
                }
                if(!(j % 8))
                    UART_WriteByte(DATACOMM_IMG_TRANS_CHL, tmp);
            }
            #if IMG_COL % 8
                // higher-bits here would be ignored, so don't worry
                UART_WriteByte(DATACOMM_IMG_TRANS_CHL, tmp);
            #endif
        }
    #endif
}
#endif

void ImgTransInRange(img_proc_struct* resultSetPtr, int16_t startIndex, int16_t endIndex) {
	for(int16_t i = startIndex - 1; i >= endIndex; i--) {
		for(int16_t j = 0; j < IMG_COL; j++) {
//            #if defined(HIGHLIGHT_MIDDLE_LINE) || defined(HIGHLIGHT_BORDER_LINE)
//            #ifdef HIGHLIGHT_MIDDLE_LINE
//            if(j == resultSetPtr->middleLine[i]) {
//                UART_WriteByte(DATACOMM_IMG_TRANS_CHL, 0x7f);
//            } else
//            #endif
//            #ifdef HIGHLIGHT_BORDER_LINE
//            if(j == resultSetPtr->leftBorder[i] || j == resultSetPtr->rightBorder[i]) {
//                UART_WriteByte(DATACOMM_IMG_TRANS_CHL, 0x3f);
//            } else
//            #endif
//            {
//            #endif
//                UART_WriteByte(DATACOMM_IMG_TRANS_CHL, IsBlack(i, j) ? IMG_BLACK : IMG_WHITE);
//            #if defined(HIGHLIGHT_MIDDLE_LINE) || defined(HIGHLIGHT_BORDER_LINE)
//            }
//            #endif
            if(j == resultSetPtr->middleLine[i]) {
                UART_WriteByte(DATACOMM_IMG_TRANS_CHL, 0x7f);
            } else if(j == resultSetPtr->leftBorder[i] || j == resultSetPtr->rightBorder[i]
                || j == IMG_COL / 2 - width__ / 2 || j == IMG_COL / 2 + width__ / 2) {
                UART_WriteByte(DATACOMM_IMG_TRANS_CHL, 0x3f);
            } else {
                UART_WriteByte(DATACOMM_IMG_TRANS_CHL, IsBlack(i, j) ? IMG_BLACK : IMG_WHITE);
            }
		}
	}
}

void ImgTransAtPreSight(img_proc_struct* resultSetPtr) {
    for(int16_t j = 0; j < IMG_COL; j++) {
        if(j == resultSetPtr->middleLine[pre_sight]) {
            UART_WriteByte(DATACOMM_IMG_TRANS_CHL, IMG_BLACK);
        } else {
            UART_WriteByte(DATACOMM_IMG_TRANS_CHL, IsBlack(pre_sight, j) ? 0x20 : 0xe0);
        }
    }
}

uint16_t CrcCheck(uint8_t *buf, uint8_t crcCnt) {
    uint16_t crc = 0xffff;
    
    for(uint8_t i = 0; i < crcCnt; i++) {
        crc ^= buf[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x01)
                crc = (crc >>1) ^ 0xa001;
            else
                crc = crc >> 1;
        }
    }
    return(crc);
}

void StateTrans(float a, float b, float c, float d) {
    float dataBuf[4] = {0};
	uint32_t temp[4] = {0};
	uint8_t frameBuf[10] = {0};
	uint16_t crc = 0;
    
	dataBuf[0] = a;
	dataBuf[1] = b;
	dataBuf[2] = c;
	dataBuf[3] = d;
    
    for(uint8_t i = 0; i < 4; i++) {
        temp[i] = (uint32_t)((int32_t)dataBuf[i]);
    }

    for(uint8_t i = 0; i < 4; i++) {
        frameBuf[i*2]   = (uint8_t)(temp[i] % 256);
        frameBuf[i*2+1] = (uint8_t)(temp[i] / 256);
    }

    crc = CrcCheck(frameBuf, 8);
    frameBuf[8] = crc % 256;
    frameBuf[9] = crc / 256;

    for(uint8_t i = 0; i < 10; i++) {
        UART_WriteByte(DATACOMM_VISUALSCOPE_CHL, frameBuf[i]);
    }
}
