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
        case 0x03:
            speed_control_speed = 95;
            pre_sight = 20; //15 //18
            break;
        case 0x02:
            speed_control_speed = 105;
            pre_sight = 21; //19 //22
            break;
        case 0x01:
            speed_control_speed = 110;
            pre_sight = 22; //21 //23
            break;
        case 0x00:
            speed_control_speed = 115;
            pre_sight = 23; //22 //24
            break;
    }
    OLEDPrintf(5, 1, "S: %d  P: %d", speed_control_speed, pre_sight);
}
