#include "MPU6050.h"

uint8_t a = 10;

HAL_StatusTypeDef MPU6050::MPU6050_WriteReg(uint8_t reg, uint8_t data)
{
    return HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);
}

HAL_StatusTypeDef MPU6050::MPU6050_ReadReg(uint8_t reg, uint8_t *data)
{
    return HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, data, 1, HAL_MAX_DELAY);
}

HAL_StatusTypeDef MPU6050::MPU6050_ReadBytes(uint8_t reg, uint8_t *data, const uint8_t len)
{
    return HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, data, len, HAL_MAX_DELAY);
}

MPU6050 & MPU6050::getinstance()
{
    static MPU6050 instance;
    return instance;
}

HAL_StatusTypeDef MPU6050::MPU6050_Init()
{
    uint8_t whoami = 0x00;

    MPU6050_ReadReg(0x75, &whoami);
    if (whoami != 0x68)
    {
        return HAL_ERROR;
    }

    // 唤醒芯片
    MPU6050_WriteReg(0x6B, 0x00);
    HAL_Delay(10);

    // 3. 配置陀螺仪量程（寄存器 0x1B）
    // 0x00: ±250°/s, 0x08: ±500°/s, 0x10: ±1000°/s, 0x18: ±2000°/s
    MPU6050_WriteReg(0x1B, 0x00);  // ±250°/s

    // 4. 配置加速度计量程（寄存器 0x1C）
    // 0x00: ±2g, 0x08: ±4g, 0x10: ±8g, 0x18: ±16g
    MPU6050_WriteReg(0x1C,0x00);  // ±2g

    // 5. 配置数字低通滤波器（寄存器 0x1A）
    // 0x00: 256Hz, 0x01: 188Hz, 0x02: 98Hz, 0x03: 42Hz, 0x04: 20Hz, 0x05: 10Hz
    MPU6050_WriteReg(0x1A, 0x03);  // 42Hz

    return HAL_OK;
}

void MPU6050::MPU6050_ReadRaw()
{
    uint8_t temp_data[14];

    if (MPU6050_ReadBytes(MPU6050_ACCEL_XOUT_H, temp_data, 14) != HAL_OK)
    {
        return;
    }
    //加速度
    mpu_data.accel_raw[0] = static_cast<int16_t>(temp_data[0] << 8 | temp_data[1]);
    mpu_data.accel_raw[1] = static_cast<int16_t>(temp_data[2] << 8 | temp_data[3]);
    mpu_data.accel_raw[2] = static_cast<int16_t>(temp_data[4] << 8 | temp_data[5]);

    //温度
    mpu_data.temp_raw = static_cast<int16_t>(temp_data[6] << 8 | temp_data[7]);

    // 陀螺仪
    mpu_data.gyro_raw[0] = static_cast<int16_t>((temp_data[8] << 8) | temp_data[9]);
    mpu_data.gyro_raw[1] = static_cast<int16_t>((temp_data[10] << 8) | temp_data[11]);
    mpu_data.gyro_raw[2] = static_cast<int16_t>((temp_data[12] << 8) | temp_data[13]);
}

void MPU6050::MPU6050_ReadData()
{
    for (uint8_t i = 0; i < 3; i ++)
    {
        mpu_data.accel_final[i] = static_cast<float>(mpu_data.accel_raw[i]) * 0.0005985502f;
        mpu_data.gyro_final[i] = static_cast<float>(mpu_data.gyro_raw[i]) * 0.00013315805450396191230191732547673f;
    }

    mpu_data.temperature = (static_cast<float>(mpu_data.temp_raw) / 340.0f) + 36.53f;
}

HAL_StatusTypeDef MPU6050::gyro_calibrate()
{
    float sum = 0;

    mpu_data.yaw_bias = 0;

    for (uint16_t i = 0; i < 1000; i++)
    {
        MPU6050_ReadRaw();
        MPU6050_ReadData();

        sum += mpu_data.gyro_final[2];
        HAL_Delay(2);
    }

    mpu_data.yaw_bias  = sum / 1000;
    return HAL_OK;
}

MPU6050::mpu6050_t &MPU6050::get_data()
{
    return mpu_data;
}

rw_lock &MPU6050::get_lock()
{
    static rw_lock _lock{};
    return _lock;
}