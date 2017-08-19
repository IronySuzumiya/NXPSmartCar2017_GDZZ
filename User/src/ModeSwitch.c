#include "ModeSwitch.h"
#include "DoubleCar.h"
#include "MainProc.h"
#include "ImgProc.h"
#include "ImgUtility.h"
#include "gpio.h"
#include "stdio.h"
#include "string.h"

bool rampOvertakingEnabled;
bool straightLineOvertakingEnabled;
bool barrierOvertakingEnabled;
bool barrierDoubleOvertakingEnabled;

#pragma diag_suppress 1293

void ModeSelect() {
    GPIO_QuickInit(MODE_SWITCH_PORT, MODE_SWITCH_PIN1, kGPIO_Mode_IPU);
    GPIO_QuickInit(MODE_SWITCH_PORT, MODE_SWITCH_PIN2, kGPIO_Mode_IPU);
    GPIO_QuickInit(MODE_SWITCH_PORT, MODE_SWITCH_PIN3, kGPIO_Mode_IPU);
    GPIO_QuickInit(MODE_SWITCH_PORT, MODE_SWITCH_PIN4, kGPIO_Mode_IPU);
    GPIO_QuickInit(MODE_SWITCH_PORT, MODE_SWITCH_PIN5, kGPIO_Mode_IPU);
    GPIO_QuickInit(MODE_SWITCH_PORT, MODE_SWITCH_PIN6, kGPIO_Mode_IPU);
    uint16_t mode = MODE_SWITCH_READ;
    double_car = !!(mode & 0x01);
    if(double_car) {
        start_line = !!(mode & 0x02);
        OLEDPrintf(5, 4, "%s %s ", "double", (start_line ? "start" : "xstart"));
        
        rampOvertakingEnabled = !!(mode & 0x04);
        straightLineOvertakingEnabled = !!(mode & 0x08);
        OLEDPrintf(5, 5, "%s %s ",
            (rampOvertakingEnabled ? "ramp" : "xramp"),
            (straightLineOvertakingEnabled ? "straight" : "xstraight"));
        
        barrierOvertakingEnabled = !!(mode & 0x10);
        leader_car = !!(mode & 0x20);
        OLEDPrintf(5, 6, "%s %s ",
            (barrierOvertakingEnabled ? "barrier" : "xbarrier"),
            (leader_car ? "leader" : "follower"));
    } else {
        start_line = !!(mode & 0x02);
        OLEDPrintf(5, 4, "%s %s ", "single", (start_line ? "start" : "xstart"));
    }
}
