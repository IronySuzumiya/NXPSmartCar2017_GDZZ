#include "DoubleCar.h"
#include "MainProc.h"
#include "gpio.h"
#include "pit.h"
#include "uart.h"
#include "ImgProc.h"
#include "ImgUtility.h"
#include "SpeedControl.h"

bool double_car;
bool final_overtaking;
int16_t ultraSonicMissingCnt;
float distanceBetweenTheTwoCars;
uint32_t time;
bool leader_car;
bool pursueing;
int32_t avg_distance_between_the_two_cars;
int32_t diff_distance_max;
bool waitForOvertaking;
bool overtaking;
bool aroundOvertaking;
int32_t waitForOvertakingTimeMax;
int32_t overtakingTime;
int32_t aroundOvertakingTimeMax;
bool holding;
int32_t holdingDistance;

#ifdef RELIABLE_CONNECTION
static uint8_t messageQueue[MESSAGE_QUEUE_SIZE];
static uint8_t *messageQueueEnd = messageQueue;
static uint8_t *messageQueueHead = messageQueue;
static bool waitingForAck;
static uint8_t lastMessage;

static void DataCommTimeOutInt(void);

static inline void SendAck() {
    UART_WriteByte(DATACOMM_DOUBLE_CAR_CHL, ACK);
}

static inline void SendLastMessage() {
    UART_WriteByte(DATACOMM_DOUBLE_CAR_CHL, lastMessage);
}

#ifdef PERIODICALLY_CHECK_MSG_QUEUE
static void DataCommSendMessageInt(void);
#endif
#endif

static void UltraSonicRecvInt(uint32_t pinxArray);
static void UltraSonicTimeOutInt(void);
static void DoubleCarMessageRecv(uint16_t byte);

void SendMessage(uint8_t message) {
    UART_WriteByte(DATACOMM_DOUBLE_CAR_CHL, message);
    #ifdef RELIABLE_CONNECTION
    waitingForAck = true;
    lastMessage = message;
    PIT_ITDMAConfig(DATACOMM_TIME_OUT_TIMER_CHL, kPIT_IT_TOF, ENABLE);
    PIT_ResetCounter(DATACOMM_TIME_OUT_TIMER_CHL);
    #endif
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
    
    #ifdef RELIABLE_CONNECTION
    /* if time out, re-send the last message */
    PIT_QuickInit(DATACOMM_TIME_OUT_TIMER_CHL, DATACOMM_TIME_OUT);
    PIT_CallbackInstall(DATACOMM_TIME_OUT_TIMER_CHL, DataCommTimeOutInt);
    PIT_ITDMAConfig(DATACOMM_TIME_OUT_TIMER_CHL, kPIT_IT_TOF, DISABLE);
    
    #ifdef PERIODICALLY_CHECK_MSG_QUEUE
    /* for every period, if not waiting for an ack, check if any message in the queue and send it */
    PIT_QuickInit(DATACOMM_SEND_MSG_TIMER_CHL, DATACOMM_SEND_MSG_PERIOD);
    PIT_CallbackInstall(DATACOMM_SEND_MSG_TIMER_CHL, DataCommSendMessageInt);
    PIT_ITDMAConfig(DATACOMM_SEND_MSG_TIMER_CHL, kPIT_IT_TOF, ENABLE);
    #endif
    #endif
}

#ifdef RELIABLE_CONNECTION
void MessageEnqueue(uint8_t message) {
    *messageQueueEnd++ = message;
    if(messageQueueEnd - messageQueue == MESSAGE_QUEUE_SIZE) {
        messageQueueEnd = messageQueue;
    }
    #ifndef PERIODICALLY_CHECK_MSG_QUEUE
    if(!waitingForAck) {
        SendMessage(*messageQueueHead++);
        if(messageQueueHead - messageQueue == MESSAGE_QUEUE_SIZE) {
            messageQueueHead = messageQueue;
        }
    }
    #endif
}
#endif

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
        case YOU_ARE_LEADER:
            leader_car = true;
            break;
        case OVERTAKING:
            overtaking = true;
            break;
        case MISSING:
            break;
        case START:
            enabled = true;
            break;
        case FINAL:
            pursueing = true;
            #ifdef FINAL_OVERTAKING
            goAlongLeft = true;
            #else
            final = true;
            #endif
            break;
        case DASH:
            finalPursueingFinished = true;
            break;
        case CROSS_ROAD:
            inCrossRoad = true;
            crossRoadDistance = 0;
            break;
        case MOVE_RIGHT_NOW:
            waitForDoubleCarAction = false;
            break;
        case HOLD:
            holding = true;
            break;
        #ifdef RELIABLE_CONNECTION
        case ACK:
            PIT_ITDMAConfig(DATACOMM_TIME_OUT_TIMER_CHL, kPIT_IT_TOF, DISABLE);
            waitingForAck = false;
            #ifndef PERIODICALLY_CHECK_MSG_QUEUE
                if(messageQueueHead != messageQueueEnd) {
                    SendMessage(*messageQueueHead++);
                    if(messageQueueHead - messageQueue == MESSAGE_QUEUE_SIZE) {
                        messageQueueHead = messageQueue;
                    }
                }
            #endif
            break;
        #endif
    }
}

#ifdef RELIABLE_CONNECTION
void DataCommTimeOutInt() {
    SendLastMessage();
}
#endif

#ifdef PERIODICALLY_CHECK_MSG_QUEUE
static void DataCommSendMessageInt() {
    if(!waitingForAck && messageQueueHead != messageQueueEnd) {
        SendMessage(*messageQueueHead++);
        if(messageQueueHead - messageQueue == MESSAGE_QUEUE_SIZE) {
            messageQueueHead = messageQueue;
        }
    }
}
#endif
