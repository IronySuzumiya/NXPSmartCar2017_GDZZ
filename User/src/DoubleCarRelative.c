#include "DoubleCarRelative.h"
#include "MainProc.h"
#include "gpio.h"
#include "pit.h"
#include "uart.h"

float distance;
uint32_t time;
bool front_car;

static void UltraSonicRecvInt(uint32_t pinxArray);

void DoubleCarRelativeInit() {
    GPIO_QuickInit(ULTRA_SONIC_RECV_PORT, ULTRA_SONIC_RECV_PIN, kGPIO_Mode_IPU);
    GPIO_CallbackInstall(ULTRA_SONIC_RECV_PORT, UltraSonicRecvInt);
    GPIO_ITDMAConfig(ULTRA_SONIC_RECV_PORT, ULTRA_SONIC_RECV_PIN, kGPIO_IT_RisingFallingEdge, ENABLE);
    
    UART_CallbackRxInstall(DATACOMM_DOUBLE_CAR_CHL, DoubleCarMessageRecv);
    UART_ITDMAConfig(DATACOMM_DOUBLE_CAR_CHL, kUART_IT_Rx, ENABLE);
    
    PIT_QuickInit(ULTRA_SONIC_TIMER_CHL, ULTRA_SONIC_TIMER_PRD);
}

void UltraSonicRecvInt(uint32_t pinxArray) {
    //if pinxArray & (1 << ULTRA_SONIC_RECV_PIN) then
    if(GPIO_ReadBit(ULTRA_SONIC_RECV_PORT, ULTRA_SONIC_RECV_PIN)) {
        PIT_ResetCounter(ULTRA_SONIC_TIMER_CHL);
    } else {
        time = (TIMER_INIT_COUNT - PIT_GetCounterValue(ULTRA_SONIC_TIMER_CHL)) / (TIMER_INIT_COUNT / ULTRA_SONIC_TIMER_PRD);
        distance = CalculateDistanceWithTime(time);
        PIT_ResetCounter(ULTRA_SONIC_TIMER_CHL);
    }
}

void DoubleCarMessageRecv(uint16_t byte) {
    switch(byte) {
        case OVER_TAKING:
            break;
    }
}
