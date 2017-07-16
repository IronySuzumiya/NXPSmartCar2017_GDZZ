#include "GearSwitch.h"
#include "SpeedControl.h"
#include "MainProc.h"
#include "gpio.h"
#include "ImgProc.h"
#include "ImgUtility.h"

void GearSelect() {
    GPIO_QuickInit(GEAR_SWITCH_PORT, GEAR_SWITCH_PIN1, kGPIO_Mode_IPU);
    GPIO_QuickInit(GEAR_SWITCH_PORT, GEAR_SWITCH_PIN2, kGPIO_Mode_IPU);
    switch(GEAR_SWITCH_READ & 0x03) {
        #if CAR_NO == 1
        
        case 0x03:
            speed_control_speed = 95;
            pre_sight = 15;
            break;
        case 0x02:
            speed_control_speed = 105;
            pre_sight = 19;
            break;
        case 0x01:
            speed_control_speed = 110;
            pre_sight = 21;
            break;
        case 0x00:
            speed_control_speed = 115;
            pre_sight = 22;
            break;
        
        #elif CAR_NO == 2
        
        case 0x03:
            speed_control_speed = 95;
            pre_sight = 15;
            break;
        case 0x02:
            speed_control_speed = 105;
            pre_sight = 19;
            break;
        case 0x01:
            speed_control_speed = 110;
            pre_sight = 21;
            break;
        case 0x00:
            speed_control_speed = 115;
            pre_sight = 22;
            break;
        
        #endif
    }
    OLEDPrintf(5, 1, "S: %d  P: %d", speed_control_speed, pre_sight);
}
