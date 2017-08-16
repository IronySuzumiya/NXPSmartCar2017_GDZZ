#ifndef _IMGPROC_H
#define _IMGPROC_H

#include "root.h"

struct _imgproc_resultset {
    int16_t leftBorder[IMG_ROW];
    int16_t rightBorder[IMG_ROW];
    int16_t middleLine[IMG_ROW];
    bool foundLeftBorder[IMG_ROW];
    bool foundRightBorder[IMG_ROW];
    int16_t leftBorderNotFoundCnt;
    int16_t rightBorderNotFoundCnt;
};

struct _imgproc;
typedef void (*_imgproc_func)(struct _imgproc *);

struct _imgproc {
    struct _camera *camera;
    _imgproc_func callback_href;
    _imgproc_func callback_vsyn;
    _imgproc_func localproc_array[IMG_ROW_INTV];
    _imgproc_func globalproc;
    _imgproc_func scan_left_border;
    _imgproc_func scan_right_border;
    _imgproc_func update_middle;
    uint8_t img_buffer_row;
    uint8_t img_real_row;
    int16_t scan_border_start_from;
    enum { Along_Middle, Along_LeftBorder, Along_RightBorder } along;
    struct _imgproc_resultset *resultset;
    
    int16_t img_hoffset;
    int16_t along_left_offset;
    int16_t along_right_offset;
};

#define IsWhite(row, col) (!camera.img_buffer[(row)][(col)])
#define IsBlack(row, col) (!IsWhite((row), (col)))

extern struct _imgproc imgproc;

#endif
