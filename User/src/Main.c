#include "MainProc.h"
#include "DataComm.h"
#include "uart.h"

int main() {
    MainInit();
    while(1) {
        if(state_trans_on) {
            StateTrans(leftPid.currentValue, rightPid.currentValue, leftPid.targetValue, rightPid.targetValue);
        }
        if(img_trans_on) {
            ImgTrans(&resultSet);
            DelayMs(50);
        }
    }
}
