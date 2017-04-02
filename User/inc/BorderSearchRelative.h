#ifndef __BORDER_SEARCH_RELATIVE
#define __BORDER_SEARCH_RELATIVE

#include "root.h"

bool LeftBorderSearch(int16_t row);
bool RightBorderSearch(int16_t row);
void MiddleLineUpdate(int16_t row);
void MiddleLineRangeUpdate(int16_t row);

#endif
