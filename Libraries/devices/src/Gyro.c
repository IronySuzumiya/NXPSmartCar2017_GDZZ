#include "Gyro.h"
#include "adc.h"

void GyroInit() {
    ADC_QuickInit(GYRO_MAP, GYRO_RESOLUTION);
}

int32_t GyroRead() {
    return ADC_QuickReadValue(GYRO_MAP);
}
