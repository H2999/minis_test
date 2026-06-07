#ifndef GPIO_WORK_MPU6050_H
#define GPIO_WORK_MPU6050_H

#include "i2c.h"
#include "mpu6050_reg.h"
#include "rw_lock.h"
#include "stm32f1xx_hal_def.h"

#define bias_yaw 0.00168458186f


class rw_lock;

class MPU6050
{
public:
    struct mpu6050_t
    {
        int16_t accel_raw[3];
        int16_t gyro_raw[3];
        int16_t temp_raw;
        float accel_final[3];
        float gyro_final[3];
        float temperature;

        float yaw_bias;
    };

    ~MPU6050() = default;

    HAL_StatusTypeDef MPU6050_Init();
    void MPU6050_ReadRaw();
    void MPU6050_ReadData();
    HAL_StatusTypeDef gyro_calibrate();
    HAL_StatusTypeDef MPU6050_ReadReg(uint8_t reg, uint8_t *data);
    HAL_StatusTypeDef MPU6050_WriteReg(uint8_t reg, uint8_t data);
    HAL_StatusTypeDef MPU6050_ReadBytes(uint8_t reg, uint8_t *data,uint8_t len);
    const mpu6050_t& get_data() const
    {
        return mpu_data;
    }

    [[nodiscard]] rw_lock &get_lock();

    MPU6050(const MPU6050 &) = delete;
    MPU6050 & operator=(const MPU6050 &) = delete;
    //预留获取实例的接口
    static MPU6050 & getinstance();
    mpu6050_t &get_data();
private:
    mpu6050_t mpu_data{};

    //单例模式 只有一个硬件mpu6050
    MPU6050() = default;
};

#endif //GPIO_WORK_MPU6050_H