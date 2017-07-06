#include "MainProc.h"
#include "DataComm.h"
#include "ImgUtility.h"
#include "ImgProc.h"
#include "DoubleCar.h"

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
        if(double_car) {
            OLEDPrintf(5, 2, "distance: %.3f", distanceBetweenTheTwoCars);
            DelayMs(500);
        }
    }
}
