#ifndef _PARAM_H
#define _PARAM_H

//NVIC
#define  HREF_IRQ  PORTB_IRQn
#define  VSYN_IRQ  PORTC_IRQn
#define  TIMR_IRQ  PIT0_IRQn
#define  ULTR_IRQ  PORTD_IRQn
#define  DCDT_IRQ  UART3_RX_TX_IRQn
#define  ULTO_IRQ  PIT1_IRQn
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
#define  IMG_READ_DELAY  115
#else
#error "not implemented"
#endif

#ifdef USE_BMP
#define  SHIFT  3
#define  MASK   0x07
#endif

//Double Car
#define  ULTRA_SONIC_TIMER_CHL  HW_PIT_CH1
#define  ULTRA_SONIC_TIME_OUT   50000
#define  TIMER_INIT_COUNT       2499991uL

//Message
#define  START                  0x01
#define  OVERTAKINGFINISHED     0x02
#define  FINAL                  0x03
#define  DASH                   0x04
#define  HOLD                   0x05

//Joystick
#define  JOYSTICK_CONFIRMING_TIMER_CHL  HW_PIT_CH3
#define  JOYSTICK_CONFIRMING_TIME       50000

//Gyro
#if CAR_NO == 1
#define GYRO_THRESHOLD 3600
#elif CAR_NO == 2
#define GYRO_THRESHOLD 2820
#endif

#endif
