#include "Joystick.h"
#include "gpio.h"
#include "SpeedControl.h"
#include "MainProc.h"

static float temp;
static int16_t index = 0;
static char *paramname[] = { "redu", "diff" };
static float *(param[]) = { &reduction_ratio, &differential_ratio };

static void JoystickInt(uint32_t pinxArray);

void JoystickInit() {
    GPIO_QuickInit(JOYSTICK_PORT, JOYSTICK_NORTH, kGPIO_Mode_IPU);
    GPIO_QuickInit(JOYSTICK_PORT, JOYSTICK_SOUTH, kGPIO_Mode_IPU);
    GPIO_QuickInit(JOYSTICK_PORT, JOYSTICK_WEST, kGPIO_Mode_IPU);
    GPIO_QuickInit(JOYSTICK_PORT, JOYSTICK_EAST, kGPIO_Mode_IPU);
    GPIO_QuickInit(JOYSTICK_PORT, JOYSTICK_MIDDLE, kGPIO_Mode_IPU);
    GPIO_CallbackInstall(JOYSTICK_PORT, JoystickInt);
    GPIO_ITDMAConfig(JOYSTICK_PORT, JOYSTICK_NORTH, kGPIO_IT_FallingEdge, ENABLE);
    GPIO_ITDMAConfig(JOYSTICK_PORT, JOYSTICK_SOUTH, kGPIO_IT_FallingEdge, ENABLE);
    GPIO_ITDMAConfig(JOYSTICK_PORT, JOYSTICK_WEST, kGPIO_IT_FallingEdge, ENABLE);
    GPIO_ITDMAConfig(JOYSTICK_PORT, JOYSTICK_EAST, kGPIO_IT_FallingEdge, ENABLE);
    GPIO_ITDMAConfig(JOYSTICK_PORT, JOYSTICK_MIDDLE, kGPIO_IT_FallingEdge, ENABLE);
    temp = reduction_ratio;
    ParamModify();
}

void JoystickInt(uint32_t pinxArray) {
    if(pinxArray & (1 << JOYSTICK_NORTH)) {
        switch(index) {
            case 0:
                temp += 0.02;
                break;
            case 1:
                temp += 0.0002;
                break;
        }
        ParamModify();
    } else if(pinxArray & (1 << JOYSTICK_SOUTH)) {
        switch(index) {
            case 0:
                temp -= 0.02;
                break;
            case 1:
                temp -= 0.0002;
                break;
        }
        ParamModify();
    } else if(pinxArray & (1 << JOYSTICK_WEST)) {
        index = Max(0, index - 1);
        temp = *(param[index]);
        ParamModify();
    } else if(pinxArray & (1 << JOYSTICK_EAST)) {
        index = Min(1, index + 1);
        temp = *(param[index]);
        ParamModify();
    } else if(pinxArray & (1 << JOYSTICK_MIDDLE)) {
        *(param[index]) = temp;
        ParamModify();
        DelayMs(100);
        OLEDPrintf(5, 4, "OK!");
    }
}

void ParamModify() {
    OLEDPrintf(5, 3, "%s: %.2f      ", paramname[index], temp);
}
