#include "DataComm.h"
#include "DoubleCar.h"
#include "ImgProc.h"
#include "uart.h"

static uint16_t CrcCheck(uint8_t *buf, uint8_t crcCnt);

void DataCommInit() {
    UART_QuickInit(DATACOMM_IMG_TRANS_MAP, DATACOMM_IMG_TRANS_BAUD);
}

void ImgTrans(uint8_t imgBuf[OV7725_H][OV7725_W / 2]) {
    UART_WriteByte(DATACOMM_IMG_TRANS_CHL, 0x01);
    UART_WriteByte(DATACOMM_IMG_TRANS_CHL, 0xfe);
    for(int16_t i = 0; i < OV7725_H; ++i) {
        for(int16_t j = MAGIC_OFFSET; j < OV7725_W / 8; ++j) {
            UART_WriteByte(HW_UART0, imgBuf[i][j]);
        }
    }
    UART_WriteByte(DATACOMM_IMG_TRANS_CHL, 0xfe);
    UART_WriteByte(DATACOMM_IMG_TRANS_CHL, 0x01);
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
