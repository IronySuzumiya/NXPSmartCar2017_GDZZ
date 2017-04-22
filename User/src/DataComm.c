#include "DataComm.h"
#include "DoubleCarRelative.h"
#include "uart.h"

bool img_trans_on, state_trans_on, mode_switch_on;

static void ImgTransInRange(img_proc_result_set_type* resultSetPtr, int16_t startIndex, int16_t endIndex);
static void ImgTransAtPreSight(img_proc_result_set_type* resultSetPtr);
static uint16_t CrcCheck(uint8_t *buf, uint8_t crcCnt);

void DataCommInit() {
    UART_QuickInit(DATACOMM_IMG_TRANS_MAP, DATACOMM_IMG_TRANS_BAUD);
    UART_CallbackRxInstall(DATACOMM_DOUBLE_CAR_CHL, DoubleCarMessageRecv);
    UART_ITDMAConfig(DATACOMM_DOUBLE_CAR_CHL, kUART_IT_Rx, ENABLE);
    //17-2-18 by Irony
    #ifdef DEBUG_ON
        UART_CallbackRxInstall(DATACOMM_IMG_TRANS_CHL, MessageParse);
        UART_ITDMAConfig(DATACOMM_IMG_TRANS_CHL, kUART_IT_Rx, ENABLE);
    #endif
}

void ImgTransOnlyBorderAndMiddleLine(int8_t* leftBorder, int8_t* middleLine, int8_t* rightBorder) {
    for(int16_t i = 0; i < IMG_ROW; i++) {
        UART_WriteByte(DATACOMM_IMG_TRANS_CHL, leftBorder[i]);
        UART_WriteByte(DATACOMM_IMG_TRANS_CHL, middleLine[i]);
        UART_WriteByte(DATACOMM_IMG_TRANS_CHL, rightBorder[i]);
    }
    UART_WriteByte(DATACOMM_IMG_TRANS_CHL, IMG_EOF);
}

void ImgTrans(img_proc_result_set_type* resultSetPtr) {
    #ifndef USE_NEW_FORMAT
        ImgTransInRange(resultSetPtr, IMG_ROW, pre_sight + 1);
        ImgTransAtPreSight(resultSetPtr);
        ImgTransInRange(resultSetPtr, pre_sight, 0);
        UART_WriteByte(DATACOMM_IMG_TRANS_CHL, IMG_EOF);
    #else
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
                if (imgBuf[i][j])
                    tmp |= 0x01;
                else
                    tmp &= ~0x01;
                for(j = 1; j < IMG_COL; ++j) {
                    if (imgBuf[i][j])
                        tmp |= 0x01 << (j % 8);
                    else
                        tmp &= ~(0x01 << (j % 8));
                    if(!(j % 8))
                        UART_WriteByte(DATACOMM_IMG_TRANS_CHL, tmp);
                }
                #if IMG_COL % 8
                    // higher-bits here would be ignored, so don't worry
                    UART_WriteByte(DATACOMM_IMG_TRANS_CHL, tmp);
                #endif
            }
        #endif
    #endif
}

void ImgTransInRange(img_proc_result_set_type* resultSetPtr, int16_t startIndex, int16_t endIndex) {
	for(int16_t i = startIndex - 1; i >= endIndex; i--) {
		for(int16_t j = 0; j < IMG_COL; j++) {
            if(j == resultSetPtr->middleLine[i] || j == resultSetPtr->leftBorder[i] || j == resultSetPtr->rightBorder[i]) { //highlight
                UART_WriteByte(DATACOMM_IMG_TRANS_CHL, 0x7f);
            } else {
                if(TstImgBufAsBitMap(i, j))
                    UART_WriteByte(DATACOMM_IMG_TRANS_CHL, IMG_BLACK);
                else
                    UART_WriteByte(DATACOMM_IMG_TRANS_CHL, IMG_WHITE);
            }
		}
	}
}

void ImgTransAtPreSight(img_proc_result_set_type* resultSetPtr) {
    for(int16_t j = 0; j < IMG_COL; j++) {
        if(j == resultSetPtr->middleLine[pre_sight])
        {
            UART_WriteByte(DATACOMM_IMG_TRANS_CHL, IMG_BLACK);
        }
        else
        {
            if(TstImgBufAsBitMap(pre_sight, j))
                UART_WriteByte(DATACOMM_IMG_TRANS_CHL, 0x20);
            else
                UART_WriteByte(DATACOMM_IMG_TRANS_CHL, 0xe0);
        }
    }
}

uint16_t CrcCheck(uint8_t *buf, uint8_t crcCnt) {
    uint16_t crc = 0xffff;
    
    for(uint8_t i = 0; i < crcCnt; i++) {
        crc ^= buf[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x01)
                crc = (crc >>1 ) ^ 0xa001;
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
