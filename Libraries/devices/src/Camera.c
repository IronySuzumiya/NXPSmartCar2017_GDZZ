#include "Camera.h"
#include "gpio.h"

static void CameraInit(struct _camera *self);
void CameraChangeStat(bool option);
static void CameraHREF(uint32_t pinxArray);
static void CameraVSYN(uint32_t pinxArray);

struct _camera camera = {
    CameraInit,
    CameraChangeStat,
    CameraHREF,
    CameraVSYN,
};

void CameraInit(struct _camera *self) {
    GPIO_QuickInit(CAMERA_HREF_PORT, CAMERA_HREF_PIN, kGPIO_Mode_IPU);
    GPIO_QuickInit(CAMERA_VSYN_PORT, CAMERA_VSYN_PIN, kGPIO_Mode_IPU);
    GPIO_CallbackInstall(CAMERA_HREF_PORT, self->callback_href);
    GPIO_CallbackInstall(CAMERA_VSYN_PORT, self->callback_vsyn);
    GPIO_QuickInit(CAMERA_DATA_PORT, CAMERA_DATA_PIN, kGPIO_Mode_IPU);
	GPIO_QuickInit(CAMERA_ODEV_PORT, CAMERA_ODEV_PIN, kGPIO_Mode_IPU);
}

void CameraChangeStat(bool option) {
    GPIO_ITDMAConfig(CAMERA_HREF_PORT, CAMERA_HREF_PIN, kGPIO_IT_RisingEdge, option);
    GPIO_ITDMAConfig(CAMERA_VSYN_PORT, CAMERA_VSYN_PIN, kGPIO_IT_RisingEdge, option);
}

void CameraHREF(uint32_t pinxArray) {
    if(pinxArray & (1 << CAMERA_HREF_PIN)) {
        imgproc.callback_href(&imgproc);
    }
}

void CameraVSYN(uint32_t pinxArray) {
    if(pinxArray & (1 << CAMERA_VSYN_PIN)) {
        imgproc.callback_vsyn(&imgproc);
    } else {
        ui.joystick.callback(&ui.joystick);
    }
}
