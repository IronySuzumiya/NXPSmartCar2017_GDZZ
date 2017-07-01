#ifndef _SPEED_CONTROL_H
#define _SPEED_CONTROL_H

#include "root.h"

#define TOO_FAR (distance > AVG_DISTANCE_BETWEEN + DIFF_DISTANCE_MAX)
#define TOO_CLOSE (distance < AVG_DISTANCE_BETWEEN - DIFF_DISTANCE_MAX)

void SpeedControlProc(int16_t leftSpeed, int16_t rightSpeed);
void SpeedTargetSet(int16_t speed, bool diff);

extern PID leftPid, rightPid;
extern int16_t leftSpeed, rightSpeed;
extern int16_t speed_control_sum_err_max;
extern bool speed_control_on;
extern int16_t speed_control_speed;
extern int16_t speed_control_acc;
extern int16_t speed_control_dec;

#endif
