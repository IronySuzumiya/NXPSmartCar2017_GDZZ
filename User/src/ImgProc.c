#include "ImgProc.h"
#include "gpio.h"
#include "DirectionControl.h"
#include "SpeedControl.h"
#include "ImgUtility.h"
#include "DataComm.h"
#include "DoubleCar.h"
#include "ModeSwitch.h"
#include "Joystick.h"

#ifdef USE_BMP
byte imgBuf[IMG_ROW][1 + IMG_COL / 8];
#else
byte imgBuf[IMG_ROW][IMG_COL];
#endif

img_proc_struct resultSet;
int16_t pre_sight;
int16_t startLinePresight;
int16_t lastAlong;
int16_t along;
bool straightLine;

static uint8_t imgBufRow = 0;
static uint8_t imgRealRow = 0;
static int16_t searchForBordersStartIndex = IMG_COL / 2;

static bool placeholder;

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
        if(imgBufRow < IMG_ROW && imgRealRow > IMG_ABDN_ROW) {
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
    } else {
        JoystickInt(pinxArray);
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
    if(along == AlongLeftBorder) {
        resultSet.foundLeftBorder[imgBufRow] =
            LeftBorderSearchFrom(imgBufRow, searchForBordersStartIndex);
        resultSet.rightBorder[imgBufRow] = resultSet.leftBorder[imgBufRow] - 40;
        ++resultSet.foundLeftBorder[imgBufRow];
    } else if(along == AlongRightBorder) {
        resultSet.foundRightBorder[imgBufRow] =
            RightBorderSearchFrom(imgBufRow, searchForBordersStartIndex);
        resultSet.leftBorder[imgBufRow] = resultSet.rightBorder[imgBufRow] + 30;
        ++resultSet.foundRightBorder[imgBufRow];
    } else if(along == AlongLeftRoad) {
        resultSet.foundLeftBorder[imgBufRow] =
            LeftBorderSearchFrom(imgBufRow, searchForBordersStartIndex);
        resultSet.rightBorder[imgBufRow] = resultSet.leftBorder[imgBufRow] + 50;
        ++resultSet.foundLeftBorder[imgBufRow];
    } else if(along == AlongRightRoad) {
        resultSet.foundRightBorder[imgBufRow] =
            RightBorderSearchFrom(imgBufRow, searchForBordersStartIndex);
        resultSet.leftBorder[imgBufRow] = resultSet.rightBorder[imgBufRow] - 60;
        ++resultSet.foundRightBorder[imgBufRow];
    } else {
        resultSet.foundLeftBorder[imgBufRow] =
            LeftBorderSearchFrom(imgBufRow, searchForBordersStartIndex);
        resultSet.foundRightBorder[imgBufRow] =
            RightBorderSearchFrom(imgBufRow, searchForBordersStartIndex);
    }
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
    int16_t middle = IMG_COL / 2;
    bool accelerate;
    if(out && enabled && !beingOvertaken && !final && IsOutOfRoad()) {
        stop = true;
    } else if(final) {
        if(finalPursueingFinished) {
            stop = (!double_car || leader_car) ? (dashDistance > 14000) : (dashDistance > 17000);
        }
        if(double_car) {
            if(leader_car) {
                if(!waitForFinalPursueing && !finalPursueingFinished) {
                    waitForFinalPursueing = true;
                } else if(finalPursueingFinished) {
                    if(!placeholder) {
                        SendMessage(DASH);
                        placeholder = true;
                    }
                }
            } else {
                if(!waitForFinalPursueing && !finalPursueingFinished) {
                    if(distanceBetweenTheTwoCars < 100) {
                        waitForFinalPursueing = true;
                        SendMessage(DASH);
                    }
                }
            }
        }
    } else {
        straightLine = IsStraightLine();
        middle = CommonAction();
    }
    
    if(direction_control_on) {
        DirectionControlProc(resultSet.middleLine, middle);
    }
    
    if(speed_control_on) {
        accelerate = straightLine;
        SpeedTargetSet(
            stop || beingOvertaken ? 0 :
//            holding ? 75 :
            double_car && leader_car && finalPursueingFinished ? 120 :
            double_car && leader_car && waitForFinalPursueing ? 0 :
            double_car && !leader_car && finalPursueingFinished ? 120 :
            double_car && !leader_car && waitForFinalPursueing ? 30 :
            leader_car && (onRamp || inStraightLine) ? 65 :
            !leader_car && (onRamp || inStraightLine) ? 85 :
            barrierOvertaking && barrierDoubleOvertakingEnabled ? 65 :
            barrierOvertaking && !barrierDoubleOvertakingEnabled && leader_car ? 65 :
            barrierOvertaking && !barrierDoubleOvertakingEnabled && !leader_car ? 85 :
            inRing || ringEndDelay ? ((!double_car || !leader_car) ? 95 : 85) :
            accelerate ? speed_control_speed * 1.1 :
            speed_control_speed,
            !(accelerate || beingOvertaken || stop)
        );
    }
}
