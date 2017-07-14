#include "ImgProc.h"
#include "gpio.h"
#include "DirectionControl.h"
#include "SpeedControl.h"
#include "ImgUtility.h"
#include "DataComm.h"
#include "DoubleCar.h"
#include "ModeSwitch.h"

#ifdef USE_BMP
byte imgBuf[IMG_ROW][1 + IMG_COL / 8];
#else
byte imgBuf[IMG_ROW][IMG_COL];
#endif

img_proc_struct resultSet;
int16_t pre_sight;
int16_t startLinePresight;

static uint8_t imgBufRow = 0;
static uint8_t imgRealRow = 0;
static int16_t searchForBordersStartIndex = IMG_COL / 2;

static void ImgProcHREF(uint32_t pinxArray);
static void ImgProcVSYN(uint32_t pinxArray);
static void ImgProc0(void);
static void ImgProc1(void);
static void ImgProc2(void);
static void ImgProc3(void);
static void ImgProcSummary(void);

static img_proc_type_array imgProc = { ImgProc0, ImgProc1, ImgProc2 ,ImgProc3 };

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
    if(pinxArray & (1 << CAMERA_HREF_PIN)) {
        if(imgBufRow < IMG_ROW && imgRealRow > IMG_ABDN_ROW)
        {
            imgProc[imgRealRow % IMG_ROW_INTV]();
        }
        ++imgRealRow;
    }
}

void ImgProcVSYN(uint32_t pinxArray) {
    if(pinxArray & (1 << CAMERA_VSYN_PIN)) {
        ImgProcSummary();
        imgRealRow = 0;
        imgBufRow = 0;
        resultSet.leftBorderNotFoundCnt = 0;
        resultSet.rightBorderNotFoundCnt = 0;
        searchForBordersStartIndex = IMG_COL / 2;
    }
}

void ImgProc0() {
    int16_t i;
    for(i = 0; i <= IMG_READ_DELAY; ++i) { } //ignore those pixels near the border
    #ifdef USE_BMP
        static byte tmpBuf[IMG_COL]; //cache
        for(i = IMG_COL - 1; i >= 0; --i) {
            tmpBuf[i] = CAMERA_DATA_READ;
            __ASM("nop");__ASM("nop");__ASM("nop");__ASM("nop");__ASM("nop");__ASM("nop");
            __ASM("nop");__ASM("nop");__ASM("nop");__ASM("nop");
        }
        for(i = IMG_COL - 1; i >= 0; --i) {
            if(tmpBuf[i])
                SetImgBufAsBitMap(imgBufRow, i);
            else
                ClrImgBufAsBitMap(imgBufRow, i);
        }
    #else
        for(i = IMG_COL - 1; i >= 0; --i) {
            imgBuf[imgBufRow][i] = CAMERA_DATA_READ;
        }
    #endif
}

void ImgProc1() {
    resultSet.foundLeftBorder[imgBufRow] =
        LeftBorderSearchFrom(imgBufRow, searchForBordersStartIndex);
    resultSet.foundRightBorder[imgBufRow] =
        RightBorderSearchFrom(imgBufRow, searchForBordersStartIndex);
}

void ImgProc2() {
    MiddleLineUpdate(imgBufRow);
    searchForBordersStartIndex = resultSet.middleLine[imgBufRow];
}

void ImgProc3() {
    CurveSlopeUpdate(imgBufRow);
    ++imgBufRow;
}

void ImgProcSummary() {
    bool doubleCarAction = false;
    int16_t middle = IMG_COL / 2;
    if(double_car) {
//        if(!firstOvertakingFinished) {
//            middle = FirstOvertakingAction();
//            doubleCarAction = true;
//        } else if(!final && startLineEnabled && IsStartLine(startLinePresight) && leader_car) {
//            SendMessage(FINAL);
//            final = true;
//        } else if(final) {
//            FinalDashAction();
//        }
    } else if(startLineEnabled && IsStartLine(startLinePresight)) {
        final = true;
        finalPursueingFinished = true;
    } else if(final && finalPursueingFinished && dashDistance > 14000) {
        stop = true;
    }
    
    if(enabled && !beingOvertaken && !final && IsOutOfRoad()) {
        stop = true;
    } else {
        if(doubleCarAction) {
            CommonAction();
        } else {
            middle = CommonAction();
        }
    }
    
    if(direction_control_on) {
        DirectionControlProc(resultSet.middleLine, middle);
    }
    
    if(speed_control_on) {
        bool accelerate = IsStraightLine();
        SpeedTargetSet(stop || beingOvertaken ? 0 :
            final && leader_car && !finalPursueingFinished ? 0 :
            final && finalPursueingFinished ? 130 :
            accelerate ? speed_control_speed * 1.1 :
            inRing || ringEndDelay ? 95 : speed_control_speed
            , !accelerate);
    }
}
