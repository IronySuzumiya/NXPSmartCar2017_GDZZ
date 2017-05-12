#include "DoubleCarRelative.h"
#include "MainProc.h"
#include "gpio.h"
#include "pit.h"
#include "uart.h"

int16_t ultraSonicMissingCnt;
float distance;
uint32_t time;
bool front_car;

static uint8_t messageQueue[MESSAGE_QUEUE_SIZE];
static uint8_t *messageQueueEnd = messageQueue;
static uint8_t *messageQueueHead = messageQueue;

static bool waitingForAck;
static uint8_t lastMessage;

static void UltraSonicRecvIntTst(uint32_t pinxArray);
static void UltraSonicRecvInt(uint32_t pinxArray);
static void UltraSonicTimeOutInt(void);
static void DoubleCarMessageRecv(uint16_t byte);
static void DataCommTimeOutInt(void);
#ifdef PERIODICALLY_CHECK_MSG_QUEUE
static void DataCommSendMessageInt(void);
#endif

static inline bool Odd(uint8_t num) {
    uint8_t cnt = 0;
    for(uint8_t i = 0; i < 7; ++i) {
        if(num & 1 << i)
            ++cnt;
    }
    return cnt % 2;
}

static inline bool Check(uint8_t num) {
    return Odd(num) ? num & 0x80 : !(num & 0x80);
}

static inline void SendAck() {
    UART_WriteByte(DATACOMM_DOUBLE_CAR_CHL, ACK | 0x80);
}

static inline void SendLastMessage() {
    UART_WriteByte(DATACOMM_DOUBLE_CAR_CHL, lastMessage);
}

static inline void SendMessage(uint8_t message) {
    message |= Odd(message) ? 0x00 : 0x80;
    UART_WriteByte(DATACOMM_DOUBLE_CAR_CHL, message);
    waitingForAck = true;
    lastMessage = message;
    PIT_ITDMAConfig(DATACOMM_TIME_OUT_TIMER_CHL, kPIT_IT_TOF, ENABLE);
}

void DoubleCarRelativeInit() {
    /* try to recv ultra sonic. if so, tell the other car it's the front one */
    GPIO_QuickInit(ULTRA_SONIC_RECV_PORT, ULTRA_SONIC_RECV_PIN, kGPIO_Mode_IPU);
    GPIO_CallbackInstall(ULTRA_SONIC_RECV_PORT, UltraSonicRecvIntTst);
    GPIO_ITDMAConfig(ULTRA_SONIC_RECV_PORT, ULTRA_SONIC_RECV_PIN, kGPIO_IT_RisingFallingEdge, ENABLE);
    
    /* used to recv messages from the other car */
    UART_CallbackRxInstall(DATACOMM_DOUBLE_CAR_CHL, DoubleCarMessageRecv);
    UART_ITDMAConfig(DATACOMM_DOUBLE_CAR_CHL, kUART_IT_Rx, ENABLE);
    
    /* ultra sonic can be missed, and `only` `for now`, do nothing if time out */
    PIT_QuickInit(ULTRA_SONIC_TIMER_CHL, ULTRA_SONIC_TIME_OUT);
    
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
}

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

void UltraSonicRecvIntTst(uint32_t pinxArray) {
    static bool ensure = false;
    if(pinxArray & (1 << ULTRA_SONIC_RECV_PIN)) {
        if(ULTRA_SONIC_RECV_READ) {
            ensure = true;
        } else {
            if(ensure) {
                MessageEnqueue(YOU_ARE_FRONT);
                front_car = false;
                
                /* begin the distance measuring */
                GPIO_CallbackInstall(ULTRA_SONIC_RECV_PORT, UltraSonicRecvInt);
                GPIO_ITDMAConfig(ULTRA_SONIC_RECV_PORT, ULTRA_SONIC_RECV_PIN, kGPIO_IT_RisingFallingEdge, ENABLE);
                
                /* if time out, send `MISSING` message */
                PIT_CallbackInstall(ULTRA_SONIC_TIMER_CHL, UltraSonicTimeOutInt);
                PIT_ITDMAConfig(ULTRA_SONIC_TIMER_CHL, kPIT_IT_TOF, ENABLE);
            }
        }
    }
}

void UltraSonicRecvInt(uint32_t pinxArray) {
    if(pinxArray & (1 << ULTRA_SONIC_RECV_PIN)) {
        if(ULTRA_SONIC_RECV_READ) {
            ultraSonicMissingCnt = 0;
            PIT_ResetCounter(ULTRA_SONIC_TIMER_CHL);
        } else {
            time = (TIMER_INIT_COUNT - PIT_GetCounterValue(ULTRA_SONIC_TIMER_CHL)) / (TIMER_INIT_COUNT / ULTRA_SONIC_TIME_OUT);
            distance = CalculateDistanceWithTime(time);
            PIT_ResetCounter(ULTRA_SONIC_TIMER_CHL);
        }
    }
}

void UltraSonicTimeOutInt() {
    ++ultraSonicMissingCnt;
    if(ultraSonicMissingCnt > 10) {
        ultraSonicMissingCnt = 0;
        MessageEnqueue(MISSING);
    }
}

void DoubleCarMessageRecv(uint16_t message) {
    if(!Check(message)) {
        return;
    }
    message &= ~0x80;
    switch(message) {
        case YOU_ARE_FRONT:
            front_car = true;
            break;
        case OVER_TAKING:
            break;
        case MISSING:
            break;
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
            return;
    }
    SendAck();
}

void DataCommTimeOutInt() {
    SendLastMessage();
}

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
