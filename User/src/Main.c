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
        if(frame_finished) {
            ImgProc();
            ImgTrans(imgBuf);
            frame_finished = false;
            GPIO_ITDMAConfig(OV7725_VSYNC_PORT, OV7725_VSYNC_PIN, kGPIO_IT_FallingEdge, ENABLE);
        }
    }
}
