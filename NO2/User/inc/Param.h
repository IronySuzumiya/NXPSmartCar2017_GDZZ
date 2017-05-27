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

//PIT
#define  PIT_CHL  HW_PIT_CH0
#define  PIT_PRD  5000

//Camera
#define  IMG_ABDN_ROW    47
#define  IMG_ROW_INTV    4

//Image Proc Bit Map Relative
/* byte is 8 (2^3) bits */
#define  SHIFT  3
/* make the after-shifted bit no more than 1 << 0x07 */
#define  MASK   0x07

#define  IMG_ROW    50
#define  IMG_COL    225
#define  WHITE_VAL  0
#define  IMG_WHITE  0xfe
#define  IMG_BLACK  0x00
#define  IMG_EOF    0xff
#define  IMG_READ_DELAY  110

//Image Proc Flag
#define  STRAIGHT_ROAD  0x0001
#define  CROSS_ROAD     0x0002

//Double Car Relative
#define  ULTRA_SONIC_TIMER_CHL  HW_PIT_CH1
#define  ULTRA_SONIC_TIME_OUT   50000
#define  TIMER_INIT_COUNT       2349993uL
#define  AVG_DISTANCE_BETWEEN   100
#define  DIFF_DISTANCE_MAX      20
#define  DATACOMM_TIME_OUT_TIMER_CHL    HW_PIT_CH2
#define  DATACOMM_TIME_OUT              5000
#define  DATACOMM_SEND_MSG_TIMER_CHL    HW_PIT_CH3
#define  DATACOMM_SEND_MSG_PERIOD       5000
//Message
#define  MESSAGE_QUEUE_SIZE     10
#define  ACK                    0x01
#define  YOU_ARE_FRONT          0x02
#define  OVER_TAKING            0x03
#define  MISSING                0x04

#endif
