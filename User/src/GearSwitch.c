#include "GearSwitch.h"
#include "SpeedControl.h"
#include "MainProc.h"
#include "gpio.h"

void GearSelect() {
    GPIO_QuickInit(GEAR_SWITCH_PORT, GEAR_SWITCH_PIN1, kGPIO_Mode_IPU);
    GPIO_QuickInit(GEAR_SWITCH_PORT, GEAR_SWITCH_PIN2, kGPIO_Mode_IPU);
    switch(GEAR_SWITCH_READ & 0x03) {
        case 0x03:
            speed_control_speed = 85;
            break;
        case 0x02:
            speed_control_speed = 90;
            break;
        case 0x01:
            speed_control_speed = 95;
            break;
        case 0x00:
            speed_control_speed = 100;
            break;
    }
    OLEDPrintf(5, 1, "Current Speed: %d", speed_control_speed);
}
