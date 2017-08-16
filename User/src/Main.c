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
        if(trans_stat) {
            StateTrans(GyroRead(), 0, 3120, 0);
        }
        if(trans_img) {
            ImgTrans(&resultSet);
        }
        if(trans_img) {
            DelayMs(50);
        } else if(trans_stat) {
            DelayMs(10);
        }
    }
}
