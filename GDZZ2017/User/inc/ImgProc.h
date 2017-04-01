#ifndef _IMGPROC_H
#define _IMGPROC_H

#include "root.h"

void ImgProcInit(void);
void SetImgBufAsBitMap(int16_t row, int16_t col);
void ClrImgBufAsBitMap(int16_t row, int16_t col);
bool TstImgBufAsBitMap(int16_t row, int16_t col);

extern byte imgBuf[IMG_ROW][1 + IMG_COL / 8];
extern int16_t dirError;
extern bool direction_control_on;
extern int16_t pre_sight;
extern img_proc_result_set_type resultSet;
extern int16_t img_border_scan_compensation;
extern int16_t wide_road_size;
extern int16_t curve_sensitivity;
extern int16_t slope_sensitivity;
extern int16_t inflexion_sensitivity;
extern int16_t cross_road_size;
extern int16_t straight_road_sensitivity;
extern int16_t straight_road_middle_area_cnt_min;
extern int16_t startline_sensitivity;
extern int16_t startline_black_tape_num;
extern int16_t mini_s_sensitivity;
extern int16_t mini_s_visual_field;

#endif
