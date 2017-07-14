#include "DoubleCar.h"
#include "MainProc.h"
#include "gpio.h"
#include "pit.h"
#include "uart.h"
#include "ImgProc.h"
#include "ImgUtility.h"
#include "SpeedControl.h"

bool double_car;
int16_t ultraSonicMissingCnt;
float distanceBetweenTheTwoCars;
uint32_t time;
bool leader_car;
int32_t avg_distance_between_the_two_cars;
int32_t diff_distance_max;
bool holding;
int32_t holdingDistance;
bool overtaking;
bool beingOvertaken;
bool alreadyReceivedOvertakingFinished;

static void UltraSonicRecvInt(uint32_t pinxArray);
static void UltraSonicTimeOutInt(void);
static void DoubleCarMessageRecv(uint16_t byte);

void SendMessage(uint8_t message) {
    UART_WriteByte(DATACOMM_DOUBLE_CAR_CHL, message);
}

void DoubleCarInit() {
    GPIO_QuickInit(ULTRA_SONIC_RECV_PORT, ULTRA_SONIC_RECV_PIN, kGPIO_Mode_IPU);
    GPIO_CallbackInstall(ULTRA_SONIC_RECV_PORT, UltraSonicRecvInt);
    GPIO_ITDMAConfig(ULTRA_SONIC_RECV_PORT, ULTRA_SONIC_RECV_PIN, kGPIO_IT_RisingFallingEdge, ENABLE);
    
    UART_QuickInit(DATACOMM_DOUBLE_CAR_MAP, DATACOMM_DOUBLE_CAR_BAUD);
    UART_CallbackRxInstall(DATACOMM_DOUBLE_CAR_CHL, DoubleCarMessageRecv);
    UART_ITDMAConfig(DATACOMM_DOUBLE_CAR_CHL, kUART_IT_Rx, ENABLE);
    
    PIT_QuickInit(ULTRA_SONIC_TIMER_CHL, ULTRA_SONIC_TIME_OUT);
    PIT_CallbackInstall(ULTRA_SONIC_TIMER_CHL, UltraSonicTimeOutInt);
    PIT_ITDMAConfig(ULTRA_SONIC_TIMER_CHL, kPIT_IT_TOF, ENABLE);
}

void UltraSonicRecvInt(uint32_t pinxArray) {
    if(pinxArray & (1 << ULTRA_SONIC_RECV_PIN)) {
        if(ULTRA_SONIC_RECV_READ) {
            ultraSonicMissingCnt = 0;
            PIT_ResetCounter(ULTRA_SONIC_TIMER_CHL);
        } else {
            time = (TIMER_INIT_COUNT - PIT_GetCounterValue(ULTRA_SONIC_TIMER_CHL)) / (TIMER_INIT_COUNT / ULTRA_SONIC_TIME_OUT);
            distanceBetweenTheTwoCars = CalculateDistanceWithTime(time);
            PIT_ResetCounter(ULTRA_SONIC_TIMER_CHL);
        }
    }
}

void UltraSonicTimeOutInt() {
    ++ultraSonicMissingCnt;
    if(ultraSonicMissingCnt > 5) {
        ultraSonicMissingCnt = 0;
        distanceBetweenTheTwoCars = avg_distance_between_the_two_cars + diff_distance_max + 233;
    }
}

void DoubleCarMessageRecv(uint16_t message) {
    switch(message) {
        case START:
            enabled = true;
            break;
        case OVERTAKINGFINISHED:
            if(!beingOvertaken) {
                alreadyReceivedOvertakingFinished = true;
            } else {
                beingOvertaken = false;
            }
            break;
        case FINAL:
            final = true;
            break;
        case DASH:
            finalPursueingFinished = true;
            break;
    }
}
