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
            speed_control_speed = 105;
            pre_sight = pre_sight_default = 22;
            break;
        case 0x02:
            speed_control_speed = 115;
            pre_sight = pre_sight_default = 24;
            break;
        case 0x01:
            speed_control_speed = 120;
            pre_sight = pre_sight_default = 25;
            break;
        case 0x00:
            speed_control_speed = 125;
            pre_sight = pre_sight_default = 26;
            break;
        
        #elif CAR_NO == 2
        
        case 0x03:
            speed_control_speed = 105;
            pre_sight = pre_sight_default = 22;
            break;
        case 0x02:
            speed_control_speed = 115;
            pre_sight = pre_sight_default = 24;
            break;
        case 0x01:
            speed_control_speed = 120;
            pre_sight = pre_sight_default = 26;
            break;
        case 0x00:
            speed_control_speed = 125;
            pre_sight = pre_sight_default = 27;
            break;
        
        #endif
    }
    OLEDPrintf(5, 1, "S: %d  P: %d", speed_control_speed, pre_sight_default);
}
