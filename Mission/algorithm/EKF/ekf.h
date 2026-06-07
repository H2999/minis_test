//
// Created by 1 on 2026/6/7.
//

#ifndef EKF_H
#define EKF_H

#include "stm32f1xx_hal.h"

class EKF
{
    struct EKF_t;

public:
    EKF();
    ~EKF();

    HAL_StatusTypeDef EKF_Init(float dt);
    void QuaternionToGravity(float gravity[3]);
    void EKF_Prediction(float gx, float gy, float gz);
    void EKF_Update(float ax, float ay, float az);
    void QuaternionToEuler(const float q[4],float *yaw,float *roll,float *pitch);
    EKF_t &get_data();

private:

    struct EKF_t
    {
        float x[6];
        float p[6][6];

        float h[3][6];

        float Q[6][6];
        float R[3][3];

        float K[6][3];

        float dt;
        float offset[4];
    };

    EKF_t *ekf_data = nullptr;

};
#endif //EKF_H