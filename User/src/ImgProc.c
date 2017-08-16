#include "ImgProc.h"
#include "gpio.h"
#include "DirectionControl.h"
#include "SpeedControl.h"
#include "ImgUtility.h"
#include "DataComm.h"
#include "DoubleCar.h"
#include "ModeSwitch.h"
#include "Joystick.h"
#include "Camera.h"

static void ImgProcHREF(struct _imgproc *self);
static void ImgProcVSYN(struct _imgproc *self);
static void ImgProc0(struct _imgproc *self);
static void ImgProc1(struct _imgproc *self);
static void ImgProc2(struct _imgproc *self);
static void ImgProc3(struct _imgproc *self);
static void ImgProcGlobal(struct _imgproc *self);

static void LeftBorderScanFrom(struct _imgproc *self);
static void RightBorderScanFrom(struct _imgproc *self);
static void MiddleLineUpdate(struct _imgproc *self);

struct _imgproc_resultset imgproc_resultset;

struct _imgproc imgproc = {
    &camera,
    ImgProcHREF,
    ImgProcVSYN,
    { ImgProc0, ImgProc1, ImgProc2 ,ImgProc3 },
    ImgProcGlobal,
    LeftBorderScanFrom,
    RightBorderScanFrom,
    MiddleLineUpdate,
    0,
    0,
    IMG_COL / 2,
    Along_Middle,
    &imgproc_resultset,
    
    IMG_READ_DELAY,
    50,
    50
};

void ImgProcHREF(struct _imgproc *self) {
    if(self->img_buffer_row < IMG_ROW && self->img_real_row > IMG_ABDN_ROW)
    {
        self->localproc_array[self->img_real_row % IMG_ROW_INTV](self);
    }
    ++self->img_real_row;
}

void ImgProcVSYN(struct _imgproc *self) {
    self->globalproc(self);
    self->img_real_row = 0;
    self->img_buffer_row = 0;
    self->resultset->leftBorderNotFoundCnt = 0;
    self->resultset->rightBorderNotFoundCnt = 0;
    self->scan_border_start_from = IMG_COL / 2;
}

void ImgProc0(struct _imgproc *self) {
    int16_t i;
    for(i = 0; i <= self->img_hoffset; ++i) { } //ignore those pixels near the border
    for(i = IMG_COL - 1; i >= 0; --i) {
        self->camera->img_buffer[self->img_buffer_row][i] = CAMERA_DATA_READ;
    }
}

void ImgProc1(struct _imgproc *self) {
    if(self->along == Along_LeftBorder) {
        self->scan_left_border(self);
        self->resultset->rightBorder[self->img_buffer_row] =
            self->resultset->leftBorder[self->img_buffer_row] + self->along_left_offset;
        ++self->resultset->foundLeftBorder[self->img_buffer_row];
        
    } else if(self->along == AlongRightBorder) {
        self->scan_right_border(self);
        self->resultset->leftBorder[self->img_buffer_row] =
            self->resultset->rightBorder[self->img_buffer_row] - self->along_right_offset;
        ++self->resultset->foundRightBorder[self->img_buffer_row];
        
    } else {
        self->scan_left_border(self);
        self->scan_right_border(self);
    }
}

void ImgProc2(struct _imgproc *self) {
    self->update_middle(self);
    self->scan_border_start_from = self->resultset->middleLine[self->img_buffer_row];
}

void ImgProc3(struct _imgproc *self) {
    ++self->img_buffer_row;
}

void ImgProcGlobal(struct _imgproc *self) {
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
        DirectionControlProc(self->resultset->middleLine, middle);
    }
    
    if(speed_control_on) {
        bool accelerate = IsStraightLine();
        SpeedTargetSet(stop || beingOvertaken ? 0 :
            final_sync && final && leader_car && !finalPursueingFinished ? 0 :
            onRamp ? 85 :
            final && finalPursueingFinished ? 130 :
            accelerate ? speed_control_speed * 1.1 :
            aroundBarrier ? speedAroundBarrier :
            inRing || ringEndDelay ? speedInRing : speed_control_speed
            , !accelerate);
    }
}

void LeftBorderScanFrom(struct _imgproc *self) {
    for(int16_t j = self->scan_border_start_from; j >= 0; --j) {
        if(IsBlack(self->img_buffer_row, j)) {
            self->resultset->leftBorder[self->img_buffer_row] = j;
            self->resultset->foundLeftBorder[self->img_buffer_row] = true;
            return;
        }
    }
    self->resultset->leftBorder[self->img_buffer_row] = -1;
    ++self->resultset->leftBorderNotFoundCnt;
    self->resultset->foundLeftBorder[self->img_buffer_row] = false;
}

void RightBorderScanFrom(struct _imgproc *self) {
    for(int16_t j = self->scan_border_start_from; j < IMG_COL; ++j) {
        if(IsBlack(self->img_buffer_row, j)) {
            self->resultset->rightBorder[self->img_buffer_row] = j;
            self->resultset->foundRightBorder[self->img_buffer_row] = true;
            return;
        }
    }
    self->resultset->rightBorder[self->img_buffer_row] = IMG_COL;
    ++self->resultset->rightBorderNotFoundCnt;
    self->resultset->foundRightBorder[self->img_buffer_row] = false;
}

void MiddleLineUpdate(struct _imgproc *self) {
    self->resultset->middleLine[self->img_buffer_row] =
        (self->resultset->leftBorder[self->img_buffer_row] + self->resultset->rightBorder[self->img_buffer_row]) / 2;
}
