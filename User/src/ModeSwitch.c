#include "ModeSwitch.h"
#include "DoubleCar.h"
#include "MainProc.h"
#include "ImgProc.h"
#include "ImgUtility.h"
#include "gpio.h"

#pragma diag_suppress 1293

void ModeSelect() {
    #if CAR_NO == 1
    GPIO_QuickInit(MODE_SWITCH_PIN123_PORT, MODE_SWITCH_PIN1, kGPIO_Mode_IPU);
    GPIO_QuickInit(MODE_SWITCH_PIN123_PORT, MODE_SWITCH_PIN2, kGPIO_Mode_IPU);
    GPIO_QuickInit(MODE_SWITCH_PIN123_PORT, MODE_SWITCH_PIN3, kGPIO_Mode_IPU);
    GPIO_QuickInit(MODE_SWITCH_PIN4_PORT, MODE_SWITCH_PIN4, kGPIO_Mode_IPU);
    #elif CAR_NO == 2
    GPIO_QuickInit(MODE_SWITCH_PORT, MODE_SWITCH_PIN1, kGPIO_Mode_IPU);
    GPIO_QuickInit(MODE_SWITCH_PORT, MODE_SWITCH_PIN2, kGPIO_Mode_IPU);
    GPIO_QuickInit(MODE_SWITCH_PORT, MODE_SWITCH_PIN3, kGPIO_Mode_IPU);
    GPIO_QuickInit(MODE_SWITCH_PORT, MODE_SWITCH_PIN4, kGPIO_Mode_IPU);
    #endif
    uint16_t mode = MODE_SWITCH_READ;
    double_car = !!(mode & 0x01);
    if(double_car) {
        if((start_line = !!(mode & 0x02))) {
            OLEDPrintf(5, 4, "Startline Enabled");
        } else {
            OLEDPrintf(5, 4, "Startline Disabled");
        }
        if((barrierOvertaking = !!(mode & 0x04))) {
            OLEDPrintf(5, 5, "Barr Ovtk Enabled");
        } else {
            OLEDPrintf(5, 5, "Barr Ovtk Disabled");
        }
        if((leader_car = !!(mode & 0x08))) {
            OLEDPrintf(5, 6, "Leader");
        } else {
            OLEDPrintf(5, 6, "Follower");
        }
    } else {
        OLEDPrintf(5, 4, "Single Mode");
    }
}
