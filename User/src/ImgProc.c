#include "ImgProc.h"
#include "gpio.h"
#include "DirectionControl.h"
#include "SpeedControl.h"
#include "ImgUtility.h"
#include "DataComm.h"
#include "DoubleCar.h"
#include "PatternMatch.h"

#ifdef USE_BMP
byte imgBuf[IMG_ROW][1 + IMG_COL / 8];
#else
byte imgBuf[IMG_ROW][IMG_COL];
#endif

bool direction_control_on;
int16_t pre_sight_default;
int16_t pre_sight;
img_proc_struct resultSet;
bool waitForOvertaking;
bool overtaking;
bool aroundOvertaking;
int32_t startDistance;
bool firstOvertakingFinished;
int32_t finalDistance;
bool final;
bool finalOvertakingFinished;
int32_t wholeDistance;
bool startLineEnabled;
bool goAlongLeft;

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

#ifdef USE_BMP
inline void SetImgBufAsBitMap(int16_t row, int16_t col) {
    imgBuf[row][col >> SHIFT] |= (1 << (col & MASK));
}

inline void ClrImgBufAsBitMap(int16_t row, int16_t col) {
    imgBuf[row][col >> SHIFT] &= ~(1 << (col & MASK));
}

inline bool TstImgBufAsBitMap(int16_t row, int16_t col) {
    return imgBuf[row][col >> SHIFT] & (1 << (col & MASK));
}
#endif

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
    for(i = 0; i <= IMG_READ_DELAY; i++) { } //ignore those pixels near the border
    #ifdef USE_BMP
        static byte tmpBuf[IMG_COL]; //cache
        for(i = IMG_COL - 1; i >= 0; i--) {
            tmpBuf[i] = CAMERA_DATA_READ;
            __ASM("nop");__ASM("nop");__ASM("nop");__ASM("nop");__ASM("nop");__ASM("nop");
            __ASM("nop");__ASM("nop");__ASM("nop");__ASM("nop");
        }
        for(i = IMG_COL - 1; i >= 0; i--) {
            if(tmpBuf[i])
                SetImgBufAsBitMap(imgBufRow, i);
            else
                ClrImgBufAsBitMap(imgBufRow, i);
        }
    #else
        for(i = IMG_COL - 1; i >= 0; i--) {
            imgBuf[imgBufRow][i] = CAMERA_DATA_READ;
        }
    #endif
}

void ImgProc1() {
    resultSet.foundLeftBorder[imgBufRow] = LeftBorderSearchFrom(imgBufRow, searchForBordersStartIndex);
    resultSet.foundRightBorder[imgBufRow] = RightBorderSearchFrom(imgBufRow, searchForBordersStartIndex);
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
    static bool stop = false;
    int16_t middle = IMG_COL / 2;
    
    #ifdef DOUBLE_CAR
    if(!firstOvertakingFinished) {
        if(leader_car) {
            if(startDistance < 20000) {
                middle -= 40;
            } else {
                waitForOvertaking = true;
                aroundOvertaking = true;
                firstOvertakingFinished = true;
            }
        } else {
            if(startDistance < 30000) {
                middle += 32;
            } else if(startDistance > 40000) {
                overtaking = true;
                SendMessage(MOVE_RIGHT_NOW);
                aroundOvertaking = true;
                firstOvertakingFinished = true;
            }
        }
    } else if(!final && startLineEnabled && IsStartLine(30)) {
        if(leader_car) {
            SendMessage(FINAL);
            final = true;
        } else {
            final = true;
        }
    } else if(final) {
        if(leader_car) {
            if(finalDistance < 11000) {
                middle -= 48;
                finalOvertakingFinished = true;
            } else if(finalDistance < 25000 && goAlongLeft) {
                middle += 32;
            } else if(finalDistance < 50000) {
                if(finalOvertakingFinished) {
                    finalOvertakingFinished = false;
                    waitForOvertaking = true;
                    aroundOvertaking = true;
                }
            } else {
                stop = true;
            }
        } else {
            if(distanceBetweenTheTwoCars < 25000 && goAlongLeft) {
                middle += 32;
            } else if(distanceBetweenTheTwoCars < 40000) {
                if(goAlongLeft) {
                    middle += 32;
                }
                if(!aroundOvertaking) {
                    overtaking = true;
                    SendMessage(OVERTAKING);
                    aroundOvertaking = true;
                }
            } else {
                stop = true;
            }
        }
    #else
    if(startLineEnabled && IsStartLine(30)) {
        final = true;
    } else if(final) {
        if(finalDistance < 11000) {
            middle -= 48;
            finalOvertakingFinished = true;
        } else if(finalDistance < 30000) {
            if(finalOvertakingFinished) {
                finalOvertakingFinished = false;
                waitForOvertaking = true;
                aroundOvertaking = true;
            }
        } else {
            stop = true;
        }
    #endif
    } else if(!aroundOvertaking && !final && IsOutOfRoad()) {
        stop = true;
    } else {
        switch(GetRoadType()) {
            case Ring:
                inRing = true;
                RingAction();
                break;
            case RingEnd:
                #ifdef DOUBLE_CAR
                    if(!aroundOvertaking) {
                        if(leader_car) {
                            waitForOvertaking = true;
                        } else if(!overtaking) {
                            overtaking = true;
                            SendMessage(OVERTAKING);
                        }
                        aroundOvertaking = true;
                    }
                #endif
                RingEndAction();
                break;
            case CrossRoad:
                inCrossRoad = true;
                crossRoadDistance = 0;
                #ifdef DOUBLE_CAR
                if(leader_car) {
                    SendMessage(CROSS_ROAD);
                }
                #endif
                break;
            case LeftCurve:
                LeftCurveAction();
                break;
            case RightCurve:
                RightCurveAction();
                break;
            case LeftBarrier:
//                middle -= 22;
                break;
            case RightBarrier:
//                middle += 22;
                break;
        }
    }
    
    if(direction_control_on) {
        DirectionControlProc(resultSet.middleLine, middle);
    }
    
    if(speed_control_on) {
        bool accelerate = IsStraightLine();
        SpeedTargetSet(stop || waitForOvertaking ? 0 :
            finalOvertakingFinished ? 30 :
            accelerate ? speed_control_speed * 1.1 :
            inRing || ringEndDelay || aroundOvertaking ? 85 : speed_control_speed
            , !stop && !waitForOvertaking && !accelerate);
    }
}
