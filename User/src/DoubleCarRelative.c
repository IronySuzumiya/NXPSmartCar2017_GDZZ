#include "DoubleCarRelative.h"
#include "MainProc.h"
#include "gpio.h"
#include "pit.h"

float distance;
uint32_t time;
bool front_car;
bool double_car;

//static ultra_sonic_time_queue_type timeQueue;

static void UltraSonicRecvInt(uint32_t pinxArray);
//static void UltraSonicTimeQueuePush(uint32_t time);
//static float UltraSonicTimeQueueGetAvg(void);

void DoubleCarRelativeInit() {
    GPIO_QuickInit(ULTRA_SONIC_RECV_PORT, ULTRA_SONIC_RECV_PIN, kGPIO_Mode_IPU);
    GPIO_CallbackInstall(ULTRA_SONIC_RECV_PORT, UltraSonicRecvInt);
    GPIO_ITDMAConfig(ULTRA_SONIC_RECV_PORT, ULTRA_SONIC_RECV_PIN, kGPIO_IT_RisingFallingEdge, ENABLE);
    
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
            if(front_car) {
                
            } else {
                motor_on = false;
            }
            break;
    }
}

//void UltraSonicTimeQueuePush(uint32_t time) {
//    timeQueue.valueBuf[timeQueue.cursor] = time;
//    timeQueue.cursor++;
//    if(timeQueue.cursor >= ULTRA_SONIC_TIME_QUEUE_LENGTH) {
//        timeQueue.cursor = 0;
//    }
//}

//float UltraSonicTimeQueueGetAvg(void) {
//    float avgTime = 0;
//    for(int16_t i = 0; i < ULTRA_SONIC_TIME_QUEUE_LENGTH; ++i) {
//        avgTime += timeQueue.valueBuf[i];
//    }
//    avgTime /= ULTRA_SONIC_TIME_QUEUE_LENGTH;
//    return avgTime;
//}
