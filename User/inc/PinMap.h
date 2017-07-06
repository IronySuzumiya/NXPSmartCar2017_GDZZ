#ifndef _PINMAP_H
#define _PINMAP_H

//Motor
#define  MOTOR_LEFT_PORT      HW_FTM0
#define  MOTOR_RIGHT_PORT     HW_FTM0
#define  MOTOR_LEFT_FOR_MAP   FTM0_CH6_PD06
#define  MOTOR_LEFT_BAK_MAP   FTM0_CH5_PD05
#define  MOTOR_RIGHT_FOR_MAP  FTM0_CH4_PD04
#define  MOTOR_RIGHT_BAK_MAP  FTM0_CH1_PC02
#define  MOTOR_LEFT_FOR_CHL   HW_FTM_CH6
#define  MOTOR_LEFT_BAK_CHL   HW_FTM_CH5
#define  MOTOR_RIGHT_FOR_CHL  HW_FTM_CH4
#define  MOTOR_RIGHT_BAK_CHL  HW_FTM_CH1

//Data Communication
#define  DATACOMM_IMG_TRANS_MAP     UART3_RX_PE05_TX_PE04
#define  DATACOMM_IMG_TRANS_CHL     HW_UART3
#define  DATACOMM_IMG_TRANS_BAUD    115200
#define  DATACOMM_VISUALSCOPE_CHL   DATACOMM_IMG_TRANS_CHL

//Camera
#define  CAMERA_HREF_PORT  HW_GPIOB
#define  CAMERA_HREF_PIN   20
#define  CAMERA_VSYN_PORT  HW_GPIOC
#define  CAMERA_VSYN_PIN   1
#define  CAMERA_DATA_PORT  HW_GPIOC
#define  CAMERA_DATA_PIN   3
#define  CAMERA_ODEV_PORT  HW_GPIOC
#define  CAMERA_ODEV_PIN   0
#define  CAMERA_DATA_READ  PCin(CAMERA_DATA_PIN)

//Encoder
#define  ENCODER_LEFT_PORT_FTM_QD   HW_FTM2
#define  ENCODER_LEFT_MAP_FTM_QD    FTM2_QD_PHA_PB18_PHB_PB19
#define  ENCODER_RIGHT_PORT_FTM_QD  HW_FTM1
#define  ENCODER_RIGHT_MAP_FTM_QD   FTM1_QD_PHA_PB00_PHB_PB01

#define  ENCODER_RIGHT_VALUE_MAP_LPTMR       LPTMR_ALT2_PC05
#define  ENCODER_RIGHT_DIRECTION_PORT_LPTMR  HW_GPIOC
#define  ENCODER_RIGHT_DIRECTION_PIN_LPTMR   10
#define  ENCODER_RIGHT_DIRECTION_READ_LPTMR  PCin(ENCODER_RIGHT_DIRECTION_PIN_LPTMR)

//Steer Actuator
#define  STEER_ACTUATOR_PORT  HW_FTM1
#define  STEER_ACTUATOR_MAP   FTM1_CH0_PA12
#define  STEER_ACTUATOR_CHL   HW_FTM_CH0

//Mode Switch
#define  MODE_SWITCH_PIN123_PORT  HW_GPIOC
#define  MODE_SWITCH_PIN1         8
#define  MODE_SWITCH_PIN2         13
#define  MODE_SWITCH_PIN3         16
#define  MODE_SWITCH_PIN4_PORT    HW_GPIOD
#define  MODE_SWITCH_PIN4         1
#define  MODE_SWITCH_READ         ((!PDin(MODE_SWITCH_PIN4) << 3u)\
                                 | (!PCin(MODE_SWITCH_PIN3) << 2u)\
                                 | (!PCin(MODE_SWITCH_PIN2) << 1u)\
                                 | (!PCin(MODE_SWITCH_PIN1)))

//Gear Switch
#define  GEAR_SWITCH_PORT  HW_GPIOE
#define  GEAR_SWITCH_PIN1  0
#define  GEAR_SWITCH_PIN2  1
#define  GEAR_SWITCH_READ  ((!PEin(GEAR_SWITCH_PIN2) << 1u)\
                          | (!PEin(GEAR_SWITCH_PIN1)))

//Double Car Relative
#define  ULTRA_SONIC_RECV_PORT     HW_GPIOA
#define  ULTRA_SONIC_RECV_PIN      19
#define  ULTRA_SONIC_RECV_READ     PAin(ULTRA_SONIC_RECV_PIN)
#define  DATACOMM_DOUBLE_CAR_MAP   DATACOMM_IMG_TRANS_MAP
#define  DATACOMM_DOUBLE_CAR_CHL   DATACOMM_IMG_TRANS_CHL
#define  DATACOMM_DOUBLE_CAR_BAUD  DATACOMM_IMG_TRANS_BAUD

//Alert
#define  BUZZLE_PORT  HW_GPIOD
#define  BUZZLE_PIN   7
#define  BUZZLE_ON    (PDout(BUZZLE_PIN) = 1)
#define  BUZZLE_OFF   (PDout(BUZZLE_PIN) = 0)

//Infrared
#define  IR_PORT  HW_GPIOD
#define  IR_PIN   0
#define  IR_READ  PDin(IR_PIN)

//Start
#define  START_PORT  HW_GPIOC
#define  START_PIN   8
#define  START_READ  PCin(START_PIN)

//OLED
#define  OELD_PORT     HW_GPIOA
#define  OELD_DC_PIN   14
#define  OELD_RST_PIN  15
#define  OELD_SDA_PIN  16
#define  OELD_SCL_PIN  17

//Joystick
#define  JOYSTICK_PORT    HW_GPIOE
#define  JOYSTICK_NORTH   16
#define  JOYSTICK_SOUTH   2
#define  JOYSTICK_WEST    6
#define  JOYSTICK_EAST    17
#define  JOYSTICK_MIDDLE  3

#endif
