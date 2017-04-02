#ifndef _PARAM_H
#define _PARAM_H

//NVIC
#define  HREF_IRQ  PORTB_IRQn
#define  VSYN_IRQ  PORTC_IRQn
#define  TIMR_IRQ  PIT0_IRQn

//PIT
#define  PIT_CHL  HW_PIT_CH0
#define  PIT_PRD  5000

//Camera
#define  IMG_ABDN_ROW    47
#define  IMG_ROW_INTV    4
#define  IMG_READ_DELAY  135 //170 for 8 nops

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

//Image Proc Flag
#define  CROSS_ROAD     0x0001
#define  STRAIGHT_ROAD  0x0002
#define  OCURVES        0x0004
#define  START_LINE     0x0008
#define  TURN_LEFT      0x0010
#define  TURN_RIGHT     0x0020
#define  MINI_S         0x0040

//Double Car Relative
#define  ULTRA_SONIC_TIMER_CHL  HW_PIT_CH1
#define  ULTRA_SONIC_TIMER_PRD  50000
#define  TIMER_INIT_COUNT       2349993uL
#define  AVG_DISTANCE_BETWEEN   100
#define  DIFF_DISTANCE_MAX      20
#define  OVER_TAKING            0x0001

#endif
