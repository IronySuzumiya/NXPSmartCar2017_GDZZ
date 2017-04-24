#include "Utility.h"

inline float Abs(float x) {
    return x >= 0 ? x : -x;
}

inline float Min(float a, float b) {
    return a > b ? b : a;
}

inline float Max(float a, float b) {
    return a > b ? a : b;
}

inline float Limit(float x, float lowerbound, float higherbound) {
    return x > higherbound ? higherbound : x < lowerbound ? lowerbound : x;
}
