#include "MainProc.h"
#include "DataComm.h"
#include "uart.h"

int main() {
//    char buf[50];
    MainInit();
    while(1) {
        if(state_trans_on) {
            StateTrans(leftPid.currentValue, rightPid.currentValue, leftPid.targetValue, rightPid.targetValue);
        }
        if(img_trans_on) {
            ImgTrans(&resultSet);
            DelayMs(50);
        }
//        sprintf(buf, "aaaaaa\r\n");
//        UART_printf(DATACOMM_IMG_TRANS_CHL, buf);
    }
}
