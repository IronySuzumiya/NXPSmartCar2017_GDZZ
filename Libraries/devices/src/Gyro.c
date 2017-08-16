#include "Gyro.h"
#include "adc.h"

static void GyroInit(struct _gyro *self);
static int32_t GyroRead(void);

struct _gyro gyro = {
    GyroInit,
    GyroRead
};

void GyroInit(struct _gyro *self) {
    ADC_QuickInit(GYRO_MAP, GYRO_RESOLUTION);
    self->work = true;
}

int32_t GyroRead() {
    return ADC_QuickReadValue(GYRO_MAP);
}
