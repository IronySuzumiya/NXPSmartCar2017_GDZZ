#include "ImgProc.h"
#include "gpio.h"
#include "DirectionControl.h"
#include "SpeedControl.h"
#include "ImgUtility.h"
#include "DataComm.h"
#include "DoubleCar.h"
#include "ModeSwitch.h"
#include "dma.h"
#include "i2c.h"
#include "MainProc.h"

uint8_t imgBuf[OV7725_H][OV7725_W / 2];
img_proc_struct resultSet;
int16_t pre_sight;
int16_t startLinePresight;
bool frame_finished;

static const struct ov7725_reg reg_tbl[] = {
    {OV7725_COM4         , 0xC1},
    {OV7725_CLKRC        , 0x00},
    {OV7725_COM2         , 0x03},
    {OV7725_COM3         , 0xD0},
    {OV7725_COM7         , 0x40},
    {OV7725_HSTART       , 0x3F},
    {OV7725_HSIZE        , 0x50},
    {OV7725_VSTRT        , 0x03},
    {OV7725_VSIZE        , 0x78},
    {OV7725_HREF         , 0x00},
    {OV7725_SCAL0        , 0x0A},
    {OV7725_AWB_Ctrl0    , 0xE0},
    {OV7725_DSPAuto      , 0xff},
    {OV7725_DSP_Ctrl2    , 0x0C},
    {OV7725_DSP_Ctrl3    , 0x00},
    {OV7725_DSP_Ctrl4    , 0x00},
    {OV7725_EXHCH        , 0x00},
    {OV7725_GAM1         , 0x0c},
    {OV7725_GAM2         , 0x16},
    {OV7725_GAM3         , 0x2a},
    {OV7725_GAM4         , 0x4e},
    {OV7725_GAM5         , 0x61},
    {OV7725_GAM6         , 0x6f},
    {OV7725_GAM7         , 0x7b},
    {OV7725_GAM8         , 0x86},
    {OV7725_GAM9         , 0x8e},
    {OV7725_GAM10        , 0x97},
    {OV7725_GAM11        , 0xa4},
    {OV7725_GAM12        , 0xaf},
    {OV7725_GAM13        , 0xc5},
    {OV7725_GAM14        , 0xd7},
    {OV7725_GAM15        , 0xe8},
    {OV7725_SLOP         , 0x20},
    {OV7725_LC_RADI      , 0x00},
    {OV7725_LC_COEF      , 0x13},
    {OV7725_LC_XC        , 0x08},
    {OV7725_LC_COEFB     , 0x14},
    {OV7725_LC_COEFR     , 0x17},
    {OV7725_LC_CTR       , 0x05},
    {OV7725_BDBase       , 0x99},
    {OV7725_BDMStep      , 0x03},
    {OV7725_SDE          , 0x04},
    {OV7725_BRIGHT       , 0x00},
    {OV7725_CNST         , 0x56},
    {OV7725_SIGN         , 0x06},
    {OV7725_UVADJ0       , 0x11},
    {OV7725_UVADJ1       , 0x02},
};

static bool in_frame;
static bool row_finished;

static void SCCBInit(void);
static void OV7725Probe(uint8_t i2c_instance);
static void OV7725SetImageSize(uint8_t i2c_instance);
static void DMAISR(void);
static void VSYNCISR(uint32_t index);
static void HREFISR(uint32_t index);

void ImgProcInit(void) {
    SCCBInit();
    
    DMA_InitTypeDef DMA_InitStruct1 = { 0 };
    
    GPIO_QuickInit(OV7725_PCLK_PORT, OV7725_PCLK_PIN, kGPIO_Mode_IPD);
    GPIO_QuickInit(OV7725_VSYNC_PORT, OV7725_VSYNC_PIN, kGPIO_Mode_IPD);
    PORT_PinPassiveFilterConfig(OV7725_VSYNC_PORT, OV7725_VSYNC_PIN, ENABLE);
    GPIO_QuickInit(OV7725_HREF_PORT, OV7725_HREF_PIN, kGPIO_Mode_IPU);
    PORT_PinPassiveFilterConfig(OV7725_HREF_PORT, OV7725_HREF_PIN, ENABLE);
    
    GPIO_CallbackInstall(OV7725_VSYNC_PORT, VSYNCISR);
    GPIO_CallbackInstall(OV7725_HREF_PORT, HREFISR);
    
    GPIO_ITDMAConfig(OV7725_VSYNC_PORT, OV7725_VSYNC_PIN, kGPIO_IT_FallingEdge, ENABLE);
    GPIO_ITDMAConfig(OV7725_HREF_PORT, OV7725_HREF_PIN, kGPIO_IT_FallingEdge, DISABLE);
    GPIO_ITDMAConfig(OV7725_PCLK_PORT, OV7725_PCLK_PIN, kGPIO_DMA_RisingEdge, ENABLE);
    
    for(int16_t i = 0; i < 8; ++i) {
        GPIO_QuickInit(OV7725_DATA_CLUSTER_PORT, OV7725_DATA_CLUSTER_OFFSET + i, kGPIO_Mode_IFT);
    }
    
    DMA_InitStruct1.chl = OV7725_DMA_CHL;
    DMA_InitStruct1.chlTriggerSource = OV7725_DMAREQ;
    DMA_InitStruct1.triggerSourceMode = kDMA_TriggerSource_Normal;
    DMA_InitStruct1.minorLoopByteCnt = 1;
    DMA_InitStruct1.majorLoopCnt = OV7725_W / 8;
    
    DMA_InitStruct1.sAddr = (uint32_t)&PTD->PDIR + OV7725_DATA_CLUSTER_OFFSET / 8;
    DMA_InitStruct1.sLastAddrAdj = 0;
    DMA_InitStruct1.sAddrOffset = 0;
    DMA_InitStruct1.sDataWidth = kDMA_DataWidthBit_8;
    DMA_InitStruct1.sMod = kDMA_ModuloDisable;
    
    DMA_InitStruct1.dAddr = NULL;
    DMA_InitStruct1.dLastAddrAdj = OV7725_W / 8;
    DMA_InitStruct1.dAddrOffset = 1;
    DMA_InitStruct1.dDataWidth = kDMA_DataWidthBit_8;
    DMA_InitStruct1.dMod = kDMA_ModuloDisable;
    
    DMA_CallbackInstall(OV7725_DMA_CHL, DMAISR);

    DMA_Init(&DMA_InitStruct1);
}

void ImgProc() {
    int16_t searchForBordersStartIndex = IMG_COL / 2;
    for(int16_t row = 0; row < OV7725_H; ++row) {
        resultSet.foundLeftBorder[row] =
            LeftBorderSearchFrom(row, searchForBordersStartIndex);
        resultSet.foundRightBorder[row] =
            RightBorderSearchFrom(row, searchForBordersStartIndex);
        MiddleLineUpdate(row);
        searchForBordersStartIndex = resultSet.middleLine[row];
        CurveSlopeUpdate(row);
    }
    bool doubleCarAction = false;
    int16_t middle = IMG_COL / 2;
    if(double_car) {
        if(final_sync) {
            if(!final && start_line && startLineEnabled && IsStartLine(startLinePresight) && leader_car) {
                SendMessage(FINAL);
                final = true;
            } else if(final) {
                FinalDashAction();
            }
        } else {
            if(!final && start_line && startLineEnabled && IsStartLine(startLinePresight)) {
                finalPursueingFinished = true;
                final = true;
            }
            if(leader_car && dashDistance > 17000) {
                stop = true;
            } else if(!leader_car && dashDistance > 13000) {
                stop = true;
            }
        }
    } else if(start_line && startLineEnabled && IsStartLine(startLinePresight)) {
        final = true;
        finalPursueingFinished = true;
    } else if(final && finalPursueingFinished && dashDistance > 14000) {
        stop = true;
    }
    
    if(out && enabled && !beingOvertaken && !final && IsOutOfRoad()) {
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
            final_sync && final && leader_car && !finalPursueingFinished ? 0 :
            final && finalPursueingFinished ? 130 :
            accelerate ? speed_control_speed * 1.1 :
            aroundBarrier ? speedAroundBarrier :
            inRing || ringEndDelay ? speedInRing : speed_control_speed
            , !accelerate);
    }
}

void SCCBInit() {
    uint32_t instance;
    instance = I2C_QuickInit(OV7725_I2C_MAP, 50000);
    OV7725Probe(instance);
    OV7725SetImageSize(instance);
}

void OV7725Probe(uint8_t i2c_instance) {
    uint8_t dummy;
    if(!SCCB_ReadSingleRegister(i2c_instance, OV7725_ADDR, OV7725_VER, &dummy)) {
        SCCB_WriteSingleRegister(i2c_instance, OV7725_ADDR, OV7725_COM7, 0x80);
        for(int16_t j = 0; j < ARRAY_SIZE(reg_tbl); j++) {
            DelayMs(1);
            SCCB_WriteSingleRegister(i2c_instance, OV7725_ADDR, reg_tbl[j].addr, reg_tbl[j].val);
        }
    }
}

void OV7725SetImageSize(uint8_t i2c_instance) {
    SCCB_WriteSingleRegister(i2c_instance, OV7725_ADDR, OV7725_HOutSize, OV7725_W / 4);
    SCCB_WriteSingleRegister(i2c_instance, OV7725_ADDR, OV7725_VOutSize, OV7725_H / 2);
}

void DMAISR(void) {
    static int16_t hcnt = 0;
    if(++hcnt == OV7725_H) {
        hcnt = 0;
        in_frame = false;
        frame_finished = true;
        GPIO_ITDMAConfig(OV7725_HREF_PORT, OV7725_HREF_PIN, kGPIO_IT_FallingEdge, DISABLE);
        GPIO_ITDMAConfig(OV7725_VSYNC_PORT, OV7725_VSYNC_PIN, kGPIO_IT_FallingEdge, DISABLE);
        DMA_ITConfig(OV7725_DMA_CHL, kDMA_IT_Major, DISABLE);
    } else {
        row_finished = true;
        DMA_SetDestAddress(OV7725_DMA_CHL, (uint32_t)imgBuf[hcnt]);
    }
}

void VSYNCISR(uint32_t index) {
    if(index & (1 << OV7725_VSYNC_PIN)) {
        if(!in_frame && !frame_finished) {
            GPIO_ITDMAConfig(OV7725_HREF_PORT, OV7725_HREF_PIN, kGPIO_IT_FallingEdge, ENABLE);
            DMA_SetDestAddress(OV7725_DMA_CHL, (uint32_t)imgBuf);
            row_finished = true;
            in_frame = true;
        }
    }
}

void HREFISR(uint32_t index) {
    if(index & (1 << OV7725_HREF_PIN)) {
        if(!row_finished) {
            DMA_DisableRequest(OV7725_DMA_CHL);
        }
        row_finished = false;
        DMA_ITConfig(OV7725_DMA_CHL, kDMA_IT_Major, ENABLE);
        DMA_EnableRequest(OV7725_DMA_CHL);
    }
}
