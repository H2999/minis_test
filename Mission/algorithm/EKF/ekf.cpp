#include "ekf.h"
#include <cmath>
#include <cstring>
#include <sys/types.h>

#include "MPU6050.h"

EKF::EKF()
{
    ekf_data =  new EKF_t{};
};

EKF::~EKF()
{
    delete ekf_data;
    if (ekf_data != nullptr)
    {
        ekf_data = nullptr;
    }
}


HAL_StatusTypeDef EKF::EKF_Init(float dt)
{
    if (ekf_data == nullptr)
    {
        return HAL_ERROR;
    }
    ekf_data->dt = dt;

    //初始的x
    for (uint8_t i = 0; i < 6; i++)
    {
        ekf_data->x[i] = 0.0f;
    }
    ekf_data->x[0] = 1.0f;

    //初始的p 描述的是对初始数据的不确定性 可以给大点 反正后面都要收敛
    for (uint8_t i = 0; i < 6; i++)
    {
        for (uint8_t j = 0; j < 6; j++)
        {
            ekf_data->p[i][j] = 0.0f;
        }
    }
    ekf_data->p[0][0] = 1.0f;
    ekf_data->p[1][1] = 1.0f;
    ekf_data->p[2][2] = 1.0f;
    ekf_data->p[3][3] = 1.0f;
    ekf_data->p[4][4] = 0.1f;
    ekf_data->p[5][5] = 0.1f;

    //初始化估计误差矩阵 对角线上描述的是x中六个变量的方差
    for (uint8_t i = 0; i < 6; i++)
    {
        for (uint8_t j = 0; j < 6; j++)
        {
            ekf_data->Q[i][j] = 0.0f;
        }
    }
    ekf_data->Q[0][0] = 0.1f;
    ekf_data->Q[1][1] = 0.1f;
    ekf_data->Q[2][2] = 0.1f;
    ekf_data->Q[3][3] = 0.1f;
    ekf_data->Q[4][4] = 0.001f;
    ekf_data->Q[5][5] = 0.001f;

    //估计误差矩阵的初始化 对角线上描述的是ax ay az方差
    for (uint8_t i = 0; i < 3; i++)
    {
        for (uint8_t j = 0; j < 3; j++)
        {
             ekf_data->R[i][j] = 0.0f;
        }
    }
    ekf_data->R[0][0] = 0.1f;
    ekf_data->R[1][1] = 0.1f;
    ekf_data->R[2][2] = 0.1f;

    return HAL_OK;
}

void EKF::QuaternionToGravity(float gravity[3])
{
    gravity[0] = 2.0f * (ekf_data->x[1] * ekf_data->x[3] - ekf_data->x[0] * ekf_data->x[2]);
    gravity[1] = 2.0f * (ekf_data->x[0] * ekf_data->x[1] + ekf_data->x[2] * ekf_data->x[3]);
    gravity[2] = 1.0f - 2.0f * (ekf_data->x[1] * ekf_data->x[1] + ekf_data->x[2] * ekf_data->x[2]);
}

void EKF::EKF_Prediction(float gx, float gy, float gz)
{
    //角速度减去偏移量得到较准的角速度
    // float wx = gx - ekf_data->x[4] - BIAS_X;
    // float wy = gy - ekf_data->x[5] - BIAS_Y;
    // float wz = gz - BIAS_Z;

    float wx = gx - ekf_data->x[4];
    float wy = gy - ekf_data->x[5];
    // float wz = gz - bias_yaw;
    float wz = gz;

    //用角速度来计算四元数(其实这步就是在求先验估计)
    float q0_dot = 0.5f * (-wx * ekf_data->x[1] - wy * ekf_data->x[2] - wz * ekf_data->x[3]);
    float q1_dot = 0.5f * (wx * ekf_data->x[0] - wy * ekf_data->x[3] + wz * ekf_data->x[2]);
    float q2_dot = 0.5f * (wx * ekf_data->x[3] + wy * ekf_data->x[0] - wz * ekf_data->x[1]);
    float q3_dot = 0.5f * (-wx * ekf_data->x[2] + wy * ekf_data->x[1] + wz * ekf_data->x[0]);

    ekf_data->x[0] +=q0_dot * ekf_data->dt;
    ekf_data->x[1] +=q1_dot * ekf_data->dt;
    ekf_data->x[2] +=q2_dot * ekf_data->dt;
    ekf_data->x[3] +=q3_dot * ekf_data->dt;

    //对四元数进行归一化
    float norm = sqrtf(ekf_data->x[0] * ekf_data->x[0] + ekf_data->x[1] * ekf_data->x[1]
        + ekf_data->x[2] * ekf_data->x[2] + ekf_data->x[3] * ekf_data->x[3]);
    if (norm > 1e-6f)
    {
        ekf_data->x[0] /= norm;
        ekf_data->x[1] /= norm;
        ekf_data->x[2] /= norm;
        ekf_data->x[3] /= norm;
    }
    //假设零漂不变 所以x[4] x[5]不变

    //预测协方差矩阵 pk = F * pk-1 * FT + Q
    //观测量有四元数和x y轴的偏移量 可以看成两个变量 6 * 6的矩阵中左上角的4 * 4矩阵是四元数的协方差 右下角的2 * 2矩阵是偏移量的协方差
    //右上角的4 * 2矩阵的每一行分别是一个四元数对偏移量求偏导 比如[0][4]是q0 对 x_bias的偏导 [0][5]是q0 对 y_bias的偏导 求偏导用更新后的四元数求即可
    float F[6][6];
    float q0 = ekf_data->x[0], q1 = ekf_data->x[1], q2 = ekf_data->x[2], q3 = ekf_data->x[3];
    float half_dt = 0.5f * ekf_data->dt;
    float halfgxdt = half_dt * wx;
    float halfgydt = half_dt * wy;
    float halfgzdt = half_dt * wz;

    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < 6; j++)
        {
            F[i][j] = 0.0f;
        }
    }

    // 左上角 4×4
    F[0][0] = 1.0f;        F[0][1] = -halfgxdt;   F[0][2] = -halfgydt;   F[0][3] = -halfgzdt;
    F[1][0] = halfgxdt;    F[1][1] = 1.0f;        F[1][2] = halfgzdt;    F[1][3] = -halfgydt;
    F[2][0] = halfgydt;    F[2][1] = -halfgzdt;   F[2][2] = 1.0f;        F[2][3] = halfgxdt;
    F[3][0] = halfgzdt;    F[3][1] = halfgydt;    F[3][2] = -halfgxdt;   F[3][3] = 1.0f;

    // 右上角 4×2
    // F[0][4] = half_dt * q1;
    // F[1][4] = -half_dt * q0;
    // F[2][4] = -half_dt * q3;
    // F[3][4] = half_dt * q2;
    //
    // F[0][5] = half_dt * q2;
    // F[1][5] = half_dt * q3;
    // F[2][5] = -half_dt * q0;
    // F[3][5] = -half_dt * q1;

    // 标准公式（通常是这样）
    F[0][4] = -half_dt * q1;   // ∂q0/∂bx
    F[1][4] =  half_dt * q0;   // ∂q1/∂bx
    F[2][4] =  half_dt * q3;   // ∂q2/∂bx
    F[3][4] = -half_dt * q2;   // ∂q3/∂bx

    F[0][5] = -half_dt * q2;   // ∂q0/∂by
    F[1][5] = -half_dt * q3;   // ∂q1/∂by
    F[2][5] =  half_dt * q0;   // ∂q2/∂by
    F[3][5] =  half_dt * q1;   // ∂q3/∂by

    // 右下角 2×2: ∂bias/∂bias = I 四元数对偏移量没有影响 所以协方差为0 只有对角线上有数据 所以右下角是单位阵
    F[4][4] = 1.0f;
    F[5][5] = 1.0f;

    //更新协方差矩阵
    float p_temp[6][6];
    float p_new[6][6];

    // 计算 p_temp = F * P
    for (uint8_t i = 0; i < 6; i++)
    {
        for (uint8_t j = 0; j < 6; j++)
        {
            p_temp[i][j] = 0.0f;
            for (uint8_t k = 0; k < 6; k++)
            {
                p_temp[i][j] += F[i][k] * ekf_data->p[k][j];
            }
        }
    }

    // 计算 p_new = P_temp * F^T + Q 别忘了加上估计误差矩阵Q 是在一个p[i][j]进行完后再加上Q[i][j]
    for (uint8_t i = 0; i < 6; i++)
    {
        for (uint8_t j = 0; j < 6; j++)
        {
            p_new[i][j] = 0.0f;
            for (uint8_t k = 0; k < 6; k++)
            {
                p_new[i][j] += p_temp[i][k] * F[j][k];
            }
            p_new[i][j] += ekf_data->Q[i][j];
        }
    }
    memcpy(ekf_data->p, p_new, sizeof(ekf_data->p));
}

void EKF::EKF_Update(float ax, float ay, float az)
{
    //对输入的角度进行归一化、
    float norm = sqrtf(ax * ax + ay * ay + az * az);
    if (norm > 1e-6f)
    {
        ax /= norm;
        ay /= norm;
        az /= norm;
    }

    float gravity_pred[3] = {0};
    //根据四元数预测重力加速度的大小
    QuaternionToGravity(gravity_pred);
    //计算残差
    float y[3];
    y[0] = ax - gravity_pred[0];
    y[1] = ay - gravity_pred[1];
    y[2] = az - gravity_pred[2];

    //计算观测矩阵H的雅可比矩阵 H维度是 3 * 6
    //gravity[0]对q0 q1 q2 q3分别求偏导 对应第一行的1-4列 x和y轴的角速度偏移量对加速度没影响 所以后两列都是0
    ekf_data->h[0][0] = -2.0f * ekf_data->x[2];
    ekf_data->h[0][1] =  2.0f * ekf_data->x[3];
    ekf_data->h[0][2] = -2.0f * ekf_data->x[0];
    ekf_data->h[0][3] =  2.0f * ekf_data->x[1];
    ekf_data->h[1][0] =  2.0f * ekf_data->x[1];
    ekf_data->h[1][1] =  2.0f * ekf_data->x[0];
    ekf_data->h[1][2] =  2.0f * ekf_data->x[3];
    ekf_data->h[1][3] =  2.0f * ekf_data->x[2];
    ekf_data->h[2][0] =  2.0f * ekf_data->x[0];
    ekf_data->h[2][1] = -2.0f * ekf_data->x[1];
    ekf_data->h[2][2] = -2.0f * ekf_data->x[2];
    ekf_data->h[2][3] =  2.0f * ekf_data->x[3];
    for (uint8_t i = 0; i < 3; i++)
    {
        for (uint8_t j = 4; j < 6; j++)
        {
            ekf_data->h[i][j] = 0.0f;
        }
    }
    //求h的转置
    float h_t[6][3]{};
    for (uint8_t i = 0; i < 6; i++)
    {
        for (uint8_t j = 0; j < 3; j++)
        {
            h_t[i][j] = ekf_data->h[j][i];
        }
    }
    //p_temp1 = p * HT
    float p_temp1[6][3];
    for (uint8_t i = 0; i < 6; i++)
    {
        for (uint8_t j = 0; j < 3; j++)
        {
            p_temp1[i][j] = 0.0f;
            for (uint8_t k = 0; k < 6; k++)
            {
                p_temp1[i][j] += ekf_data->p[i][k] * h_t[k][j];
            }
        }
    }
    //p_temp2 = H * p
    float p_temp2[3][6];
    for (uint8_t i = 0; i < 3; i++)
    {
        for (uint8_t j = 0; j < 6; j++)
        {
            p_temp2[i][j] = 0.0f;
            for (uint8_t k = 0; k < 6; k++)
            {
                p_temp2[i][j] += ekf_data->h[i][k] * ekf_data->p[k][j];
            }
        }
    }
    //p_temp3 = p_temp2 * HT + R
    float p_temp3[3][3];
    for (uint8_t i = 0; i < 3; i++)
    {
        for (uint8_t j = 0; j < 3; j++)
        {
            p_temp3[i][j] = 0.0f;
            for (uint8_t k = 0; k < 6; k++)
            {
                p_temp3[i][j] += p_temp2[i][k] * h_t[k][j];
            }
            p_temp3[i][j] += ekf_data->R[i][j];
        }
    }

    float det = p_temp3[0][0] * (p_temp3[1][1]*p_temp3[2][2] - p_temp3[1][2]*p_temp3[2][1]) -
               p_temp3[0][1] * (p_temp3[1][0]*p_temp3[2][2] - p_temp3[1][2]*p_temp3[2][0]) +
               p_temp3[0][2] * (p_temp3[1][0]*p_temp3[2][1] - p_temp3[1][1]*p_temp3[2][0]);

    if (det < 1e-6f) det = 1e-6f;

    float inv_s[3][3];
    inv_s[0][0] = (p_temp3[1][1]*p_temp3[2][2] - p_temp3[1][2]*p_temp3[2][1]) / det;
    inv_s[0][1] = (p_temp3[0][2]*p_temp3[2][1] - p_temp3[0][1]*p_temp3[2][2]) / det;
    inv_s[0][2] = (p_temp3[0][1]*p_temp3[1][2] - p_temp3[0][2]*p_temp3[1][1]) / det;
    inv_s[1][0] = (p_temp3[1][2]*p_temp3[2][0] - p_temp3[1][0]*p_temp3[2][2]) / det;
    inv_s[1][1] = (p_temp3[0][0]*p_temp3[2][2] - p_temp3[0][2]*p_temp3[2][0]) / det;
    inv_s[1][2] = (p_temp3[0][2]*p_temp3[1][0] - p_temp3[0][0]*p_temp3[1][2]) / det;
    inv_s[2][0] = (p_temp3[1][0]*p_temp3[2][1] - p_temp3[1][1]*p_temp3[2][0]) / det;
    inv_s[2][1] = (p_temp3[0][1]*p_temp3[2][0] - p_temp3[0][0]*p_temp3[2][1]) / det;
    inv_s[2][2] = (p_temp3[0][0]*p_temp3[1][1] - p_temp3[0][1]*p_temp3[1][0]) / det;

    //K = p_temp1 * inv_s
    for (uint8_t i = 0; i < 6; i++)
    {
        for (uint8_t j = 0; j < 3; j++)
        {
            ekf_data->K[i][j] = 0.0f;
            for (uint8_t k = 0; k < 3; k++)
            {
                ekf_data->K[i][j] += p_temp1[i][k] * inv_s[k][j];
            }
        }
    }

    //利用卡尔曼增益更新数据
    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            ekf_data->x[i] += ekf_data->K[i][j] * y[j];
        }
    }
    //归一化四元数
    float norm_q = sqrtf(ekf_data->x[0] * ekf_data->x[0] + ekf_data->x[1] * ekf_data->x[1]
        + ekf_data->x[2] * ekf_data->x[2] + ekf_data->x[3] * ekf_data->x[3]);
    if (norm_q > 1e-6f)
    {
        ekf_data->x[0] /= norm_q;
        ekf_data->x[1] /= norm_q;
        ekf_data->x[2] /= norm_q;
        ekf_data->x[3] /= norm_q;
    }


    // 计算 P_new = (I - KH) * P
    float I_KH[6][6];
    // 先计算 I - K*H
    for (uint8_t i = 0; i < 6; i++)
    {
        for (uint8_t j = 0; j < 6; j++)
        {
            float kh_sum = 0.0f;
            for (int k = 0; k < 3; k++)
            {
                kh_sum += ekf_data->K[i][k] * ekf_data->h[k][j];
            }
            I_KH[i][j] = (i == j ? 1.0f : 0.0f) - kh_sum;
        }
    }
    // 再乘 P
    float P_new[6][6];
    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < 6; j++)
        {
            P_new[i][j] = 0.0f;
            for (int k = 0; k < 6; k++)
            {
                P_new[i][j] += I_KH[i][k] * ekf_data->p[k][j];
            }
        }
    }
    memcpy(ekf_data->p, P_new, sizeof(ekf_data->p));
}

void EKF::QuaternionToEuler(const float q[4],float *yaw,float *roll,float *pitch)
{
    *yaw = atan2f(2.0f * (q[0] * q[3] + q[1] * q[2]), 2.0f * (q[0] * q[0] + q[1] * q[1]) - 1.0f) * 57.295779513f;
    *roll = atan2f(2.0f * (q[0] * q[1] + q[2] * q[3]), 2.0f * (q[0] * q[0] + q[3] * q[3]) - 1.0f) * 57.295779513f;
    *pitch = asinf(-2.0f * (q[1] * q[3] - q[0] * q[2])) * 57.295779513f;
}

EKF::EKF_t &EKF::get_data()
{
    return *ekf_data;
}
