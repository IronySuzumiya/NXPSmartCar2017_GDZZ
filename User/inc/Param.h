#ifndef _PARAM_H
#define _PARAM_H

//NVIC
#define  HREF_IRQ  PORTB_IRQn
#define  VSYN_IRQ  PORTC_IRQn
#define  TIMR_IRQ  PIT0_IRQn
#define  ULTR_IRQ  PORTD_IRQn
#define  DCDT_IRQ  UART3_RX_TX_IRQn
#define  ULTO_IRQ  PIT1_IRQn
#define  DCTO_IRQ  PIT2_IRQn
#define  JYSK_IRQ  PIT3_IRQn

//PIT
#define  PIT_CHL  HW_PIT_CH0
#define  PIT_PRD  5000

//Camera
#define  IMG_ABDN_ROW  47
#define  IMG_ROW_INTV  4
#define  IMG_ROW       50
#define  IMG_COL       225
#define  WHITE_VAL     0
#define  BLACK_VAL     1
#define  IMG_WHITE     0xfe
#define  IMG_BLACK     0x00
#define  IMG_EOF       0xff

#if CAR_NO == 1
#define  IMG_READ_DELAY  130
#elif CAR_NO == 2
#define  IMG_READ_DELAY  110
#else
#error "not implemented"
#endif

#ifdef USE_BMP
#define  SHIFT  3
#define  MASK   0x07
#endif

//Double Car
#define  ULTRA_SONIC_TIMER_CHL              HW_PIT_CH1
#define  ULTRA_SONIC_TIME_OUT               50000
#define  TIMER_INIT_COUNT                   2499991uL
#define  DATACOMM_TIME_OUT_TIMER_CHL        HW_PIT_CH2
#define  DATACOMM_TIME_OUT                  5000
#define  DATACOMM_SEND_MSG_TIMER_CHL        HW_PIT_CH3
#define  DATACOMM_SEND_MSG_PERIOD           5000

//Message
#define  MESSAGE_QUEUE_SIZE     10
#define  ACK                    0x01
#define  YOU_ARE_LEADER         0x02
#define  OVERTAKING             0x03
#define  MISSING                0x04
#define  START                  0x05
#define  FINAL                  0x06
#define  CROSS_ROAD             0x07
#define  MOVE_RIGHT_NOW         0x08
#define  DASH                   0x09
#define  HOLD                   0x0A

//Joystick
//#warning "there is a conflict between doublecar-datacomm and joystick-confirming-timer on pit-ch3"
#define  JOYSTICK_CONFIRMING_TIMER_CHL  HW_PIT_CH3
#define  JOYSTICK_CONFIRMING_TIME       50000

#endif
