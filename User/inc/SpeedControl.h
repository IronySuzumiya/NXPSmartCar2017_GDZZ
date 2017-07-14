#ifndef _SPEED_CONTROL_H
#define _SPEED_CONTROL_H

#include "root.h"

#define TOO_FAR (distanceBetweenTheTwoCars > avg_distance_between_the_two_cars + diff_distance_max)
#define EXT_FAR (distanceBetweenTheTwoCars > avg_distance_between_the_two_cars + 2 * diff_distance_max)
#define TOO_CLOSE (distanceBetweenTheTwoCars < avg_distance_between_the_two_cars - diff_distance_max)
#define EXT_CLOSE (distanceBetweenTheTwoCars < avg_distance_between_the_two_cars - 2 * diff_distance_max)

void SpeedControlProc(int16_t leftSpeed, int16_t rightSpeed);
void SpeedTargetSet(int16_t speed, bool diff);

extern PID leftPid, rightPid;
extern int16_t leftSpeed, rightSpeed;
extern int16_t speed_control_sum_err_max;
extern bool speed_control_on;
extern int16_t speed_control_speed;
extern int16_t speed_control_acc;
extern int16_t speed_control_dec;
extern float reduction_ratio;
extern float differential_ratio;
extern bool stop;
extern int16_t barrierSpeed;

#endif
