#ifndef _PARAM_H
#define _PARAM_H

//NVIC
#define  HREF_IRQ  PORTE_IRQn
#define  VSYN_IRQ  PORTC_IRQn
#define  TIMR_IRQ  PIT0_IRQn
#define  ULTR_IRQ  PORTD_IRQn
#define  DCDT_IRQ  UART3_RX_TX_IRQn
#define  ULTO_IRQ  PIT1_IRQn
#define  JYSK_IRQ  PIT3_IRQn

//PIT
#define  PIT_CHL   HW_PIT_CH0
#define  PIT_PRD   5000

//Camera
#define  OV7725_W       80
#define  OV7725_H       60
#define  IMG_ROW        OV7725_H
#define  IMG_COL        OV7725_W
#define  OV7725_DMAREQ  PORTC_DMAREQ
#define  MAGIC_OFFSET   2

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

#endif
