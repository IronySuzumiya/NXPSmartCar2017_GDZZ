#ifndef __PATTERN_MATCH_H
#define __PATTERN_MATCH_H

#include "root.h"

void CrossRoadJudge(int16_t row);
void StraightRoadJudge(int16_t* middleLine);
void MiniSJudge(int16_t* middleLine, int16_t minRow, int16_t maxRow);
void StartLineJudge(int16_t row);
void CurveJudge(int16_t row);

#endif
