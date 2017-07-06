#include "ModeSwitch.h"
#include "DoubleCar.h"
#include "MainProc.h"
#include "gpio.h"

static void Mode0(void);
static void Mode1(void);
static void Mode2(void);
static void Mode3(void);
static void Mode4(void);

static mode_type_array mode = { Mode0, Mode1, Mode2, Mode3, Mode4, NULL };

void ModeSelect() {
    GPIO_QuickInit(MODE_SWITCH_PIN12_PORT, MODE_SWITCH_PIN1, kGPIO_Mode_IPU);
    GPIO_QuickInit(MODE_SWITCH_PIN12_PORT, MODE_SWITCH_PIN2, kGPIO_Mode_IPU);
    GPIO_QuickInit(MODE_SWITCH_PIN3_PORT, MODE_SWITCH_PIN3, kGPIO_Mode_IPU);
    mode[MODE_SWITCH_READ]();
}

void Mode0() {
    double_car = false;
    OLEDPrintf(5, 4, "Single Car");
}

void Mode1() {
    double_car = true;
    OLEDPrintf(5, 4, "Double Car");
}

void Mode2() {
    
}

void Mode3() {
    
}

void Mode4() {
    
}
