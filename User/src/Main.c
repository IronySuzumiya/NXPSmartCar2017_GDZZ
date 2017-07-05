#include "MainProc.h"
#include "DataComm.h"
#include "ImgUtility.h"
#include "ImgProc.h"

int main() {
    MainInit();
    while(1) {
        #ifdef USE_STAT_TRANS
            StateTrans(leftPid.currentValue, rightPid.currentValue, ringDistance, rightPid.targetValue);
        #endif
        #ifdef USE_IMG_TRANS
            ImgTrans(&resultSet);
            DelayMs(50);
        #endif
    }
}
