#include "MainProc.h"
#include "DataComm.h"
#include "ImgUtility.h"
#include "ImgProc.h"
#include "DoubleCar.h"
#include "Gyro.h"
#include "gpio.h"

int main() {
    MainInit();
    while(1) {
        #ifdef USE_STAT_TRANS
            StateTrans(GyroRead(), 0, 3000, 0);
            DelayMs(10);
        #endif
        #ifdef USE_IMG_TRANS
            ImgTrans(&resultSet);
            DelayMs(50);
        #endif
    }
}
