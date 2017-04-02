#include "MathPropertiesAnalyze.h"
#include "ImgProc.h"

void CurveSlopeUpdate(int16_t row) {
    if(row >= slope_sensitivity) {
        float leftSlopeX = 0, leftSlopeA = 0, leftSlopeB = 0;
        float rightSlopeX = 0, rightSlopeA = 0, rightSlopeB = 0;
        float middleSlopeX = 0, middleSlopeA = 0, middleSlopeB = 0;
        for(int16_t i = row - slope_sensitivity; i <= row; ++i) {
            leftSlopeX += resultSet.leftBorder[i];
            leftSlopeA += i * resultSet.leftBorder[i];
            rightSlopeX += resultSet.rightBorder[i];
            rightSlopeA += i * resultSet.rightBorder[i];
            middleSlopeX += resultSet.middleLine[i];
            middleSlopeA += i * resultSet.middleLine[i];
        }
        leftSlopeB = (row - slope_sensitivity / 2) * leftSlopeX;
        rightSlopeB = (row - slope_sensitivity / 2) * rightSlopeX;
        middleSlopeB = (row - slope_sensitivity / 2) * middleSlopeX;
        
        resultSet.leftSlope[row] = (leftSlopeA - leftSlopeB) / 10.0;
        resultSet.leftZero[row] = (leftSlopeX / 5) - resultSet.leftSlope[row] * (row - slope_sensitivity / 2);
        resultSet.rightSlope[row] = (rightSlopeA - rightSlopeB) / 10.0;
        resultSet.rightZero[row] = (rightSlopeX / 5) - resultSet.rightSlope[row] * (row - slope_sensitivity / 2);
        resultSet.middleSlope[row] = (middleSlopeA - middleSlopeB) / 10.0;
        resultSet.middleZero[row] = (middleSlopeX / 5) - resultSet.middleSlope[row] * (row - slope_sensitivity / 2);
    } else {
        resultSet.leftSlope[row] = 0;
        resultSet.leftZero[row] = 0;
        resultSet.rightSlope[row] = 0;
        resultSet.rightZero[row] = IMG_COL - 1;
        resultSet.middleSlope[row] = 0;
        resultSet.middleZero[row] = IMG_COL / 2;
    }
}

void CurveTrendUpdate(int16_t row) {
    if(row >= inflexion_sensitivity) {
        resultSet.leftTrend[row] = resultSet.leftBorder[row] - resultSet.leftBorder[row - inflexion_sensitivity];
        resultSet.rightTrend[row] = resultSet.rightBorder[row] - resultSet.rightBorder[row - inflexion_sensitivity];
    } else {
        resultSet.leftTrend[row] = 0;
        resultSet.rightTrend[row] = 0;
    }
}
