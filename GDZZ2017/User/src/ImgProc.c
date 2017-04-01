#include "ImgProc.h"
#include "gpio.h"
#include "DirectionControl.h"
#include "SpeedControl.h"
#include "ImgUtility.h"
#include "DataComm.h"

byte imgBuf[IMG_ROW][1 + IMG_COL / 8]; // Important extra 1 byte against overflow
int16_t dirError;
bool direction_control_on;
int16_t pre_sight;
img_proc_result_set_type resultSet;
int16_t img_border_scan_compensation;
int16_t wide_road_size;
int16_t curve_sensitivity;
int16_t slope_sensitivity;
int16_t inflexion_sensitivity;
int16_t cross_road_size;
int16_t straight_road_sensitivity;
int16_t straight_road_middle_area_cnt_min;
int16_t startline_sensitivity;
int16_t startline_black_tape_num;
int16_t mini_s_sensitivity;
int16_t mini_s_visual_field;

static uint8_t imgBufRow = 0;
static uint8_t imgRealRow = 0;

static void ImgProcHREF(uint32_t pinxArray);
static void ImgProcVSYN(uint32_t pinxArray);
static void ImgProc0(void);
static void ImgProc1(void);
static void ImgProc2(void);
static void ImgProcSummary(void);

static img_proc_type_array imgProc = { ImgProc0, ImgProc1, ImgProc2 };

inline void SetImgBufAsBitMap(int16_t row, int16_t col) {
    imgBuf[row][col >> SHIFT] |= (1 << (col & MASK));
}

inline void ClrImgBufAsBitMap(int16_t row, int16_t col) {
    imgBuf[row][col >> SHIFT] &= ~(1 << (col & MASK));
}

inline bool TstImgBufAsBitMap(int16_t row, int16_t col) {
    return imgBuf[row][col >> SHIFT] & (1 << (col & MASK));
}

void ImgProcInit(void) {
    GPIO_QuickInit(CAMERA_HREF_PORT, CAMERA_HREF_PIN, kGPIO_Mode_IPU);
    GPIO_QuickInit(CAMERA_VSYN_PORT, CAMERA_VSYN_PIN, kGPIO_Mode_IPU);
    GPIO_CallbackInstall(CAMERA_HREF_PORT, ImgProcHREF);
    GPIO_CallbackInstall(CAMERA_VSYN_PORT, ImgProcVSYN);
    GPIO_ITDMAConfig(CAMERA_HREF_PORT, CAMERA_HREF_PIN, kGPIO_IT_RisingEdge, ENABLE);
    GPIO_ITDMAConfig(CAMERA_VSYN_PORT, CAMERA_VSYN_PIN, kGPIO_IT_RisingEdge, ENABLE);
    
    GPIO_QuickInit(CAMERA_DATA_PORT, CAMERA_DATA_PIN, kGPIO_Mode_IPU);
	GPIO_QuickInit(CAMERA_ODEV_PORT, CAMERA_ODEV_PIN, kGPIO_Mode_IPU);
}

void ImgProcHREF(uint32_t pinxArray) {
    //if pinxArray & (1 << CAMERA_HREF_PIN) then
    if(imgBufRow < IMG_ROW && imgRealRow > IMG_ABDN_ROW)
    {
        imgProc[imgRealRow % IMG_ROW_INTV]();
    }
    imgRealRow++;
}

void ImgProcVSYN(uint32_t pinxArray) {
    //if pinxArray & (1 << CAMERA_VSYN_PIN) then
    ImgProcSummary();
    imgRealRow = 0;
    imgBufRow = 0;
    resultSet.imgProcFlag = 0;
    resultSet.middleLineMaxRow = 0;
    resultSet.middleLineMinRow = 0;
}

void ImgProc0() {
    int16_t i;
    static byte tmpBuf[IMG_COL]; //cache
    for(i = 0; i <= IMG_READ_DELAY; i++); //ignore points near the border
    for(i = IMG_COL - 1; i >= 0; i--) {
        tmpBuf[i] = CAMERA_DATA_READ;
        __ASM("nop");__ASM("nop");__ASM("nop");__ASM("nop");__ASM("nop");__ASM("nop");__ASM("nop");__ASM("nop");
    }
    for(i = IMG_COL - 1; i >= 0; i--) {
        if(tmpBuf[i])
            SetImgBufAsBitMap(imgBufRow, i);
        else
            ClrImgBufAsBitMap(imgBufRow, i);
    }
}

void ImgProc1() {
    resultSet.foundLeftBorder[imgBufRow] = LeftBorderSearch(imgBufRow);
    resultSet.foundRightBorder[imgBufRow] = RightBorderSearch(imgBufRow);
    MiddleLineUpdate(imgBufRow);
}

void ImgProc2() {
    MiddleLineRangeUpdate(imgBufRow);
    imgBufRow++;
}

void ImgProcSummary() {
    StartLineJudge(pre_sight - 10);
    StraightRoadJudge(resultSet.middleLine);
    MiniSJudge(resultSet.middleLine, resultSet.middleLineMinRow, resultSet.middleLineMaxRow);
    if(resultSet.imgProcFlag & (START_LINE | MINI_S)) {
        BUZZLE_ON;
    } else {
        BUZZLE_OFF;
//        CurveJudge(pre_sight);
    }
    if(direction_control_on) {
        DirectionControlProc(resultSet.middleLine);
    }
    if(speed_control_on) {
        SpeedTargetSet(resultSet.imgProcFlag);
    }
}
