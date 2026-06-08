#include "SPL.h"
#include "i2c.h"
#include "spl_reg.h"
#include "stm32f1xx_hal_i2c.h"

HAL_StatusTypeDef SPL06::SPL_WriteReg(uint8_t reg, uint8_t data)
{
    return HAL_I2C_Mem_Write(&hi2c2,SPL_ADDRESS,reg,I2C_MEMADD_SIZE_8BIT,&data,1,HAL_MAX_DELAY);
}

HAL_StatusTypeDef SPL06::SPL_ReadReg(uint8_t reg, uint8_t *data)
{
    return HAL_I2C_Mem_Read(&hi2c2,SPL_ADDRESS,reg,I2C_MEMADD_SIZE_8BIT,data,1,HAL_MAX_DELAY);
}

HAL_StatusTypeDef SPL06::SPL_ReadBytes(uint8_t reg, uint8_t *data,uint8_t len)
{
    return HAL_I2C_Mem_Read(&hi2c2,SPL_ADDRESS,reg,I2C_MEMADD_SIZE_8BIT,data,len,HAL_MAX_DELAY);
}

HAL_StatusTypeDef SPL06::SPL_init()
{
    if (SPL_WriteReg(SPL_PRESSURE_CFG,0x26) != HAL_OK ||
    SPL_WriteReg(SPL_TEMPERATURE_CFG,0xA0) != HAL_OK ||
    SPL_WriteReg(SPL_MEAS_CFG,0x07) != HAL_OK ||
    SPL_WriteReg(SPL_CFG_REG,0x04) != HAL_OK)
    {
        return HAL_ERROR;
    }
    return HAL_OK;
}

SPL06 &SPL06::getinstance()
{
    static SPL06 instance;
    return instance;
}

SPL06::SPL06()
{
    SPL_data = new SPL_t;
}

SPL06::~SPL06()
{
    if (SPL_data != nullptr)
    {
        delete SPL_data;
        SPL_data = nullptr;
    }
}

void SPL06::get_calibration_data()
{
    uint8_t calibration_data[18] = {0};
    uint16_t temp_data[2] = {0};
    uint32_t pressure_data[2] = {0};
    HAL_StatusTypeDef status = HAL_ERROR;

    status = SPL_ReadBytes(SPL_Calibration_c0,calibration_data,18);
    if (status != HAL_OK)
    {
        return;
    }

    temp_data[0] = (static_cast<uint16_t>(calibration_data[0]) << 4)
                    | (static_cast<uint16_t>(calibration_data[1] & 0xF0) >> 4);
    SPL_data->calibration_data.temperature_calibration.c0 = static_cast<int16_t>(temp_data[0] << 4) >> 4;

    temp_data[1] = (static_cast<uint16_t>(calibration_data[1] & 0x0F) << 8) | calibration_data[2];
    SPL_data->calibration_data.temperature_calibration.c1 = static_cast<int16_t>(temp_data[1] << 4) >> 4;

    pressure_data[0] = (static_cast<uint32_t>(calibration_data[3]) << 12)
        | (static_cast<uint32_t>(calibration_data[4]) << 4) | (static_cast<uint32_t>(calibration_data[5] & 0xF0) >> 4);
    SPL_data->calibration_data.pressure_calibration.c00 = static_cast<int32_t>(pressure_data[0] << 12) >> 12;

    pressure_data[1] = (static_cast<uint32_t>(calibration_data[5] & 0x0F) << 16) |
                (static_cast<uint32_t>(calibration_data[6]) << 8) | static_cast<uint32_t>(calibration_data[7]);
    SPL_data->calibration_data.pressure_calibration.c10 = static_cast<int32_t>(pressure_data[1] << 12) >> 12;

    SPL_data->calibration_data.pressure_calibration.c01 = static_cast<int16_t>(calibration_data[8] << 8) | calibration_data[9];
    SPL_data->calibration_data.pressure_calibration.c11 = static_cast<int16_t>((calibration_data[10] << 8) | calibration_data[11]);
    SPL_data->calibration_data.pressure_calibration.c20 = static_cast<int16_t>(calibration_data[12] << 8) | calibration_data[13];
    SPL_data->calibration_data.pressure_calibration.c21 = static_cast<int16_t>(calibration_data[14] << 8) | calibration_data[15];
    SPL_data->calibration_data.pressure_calibration.c30 = static_cast<int16_t>(calibration_data[16] << 8) | calibration_data[17];
}

void SPL06::get_pressure_temp_raw_data()
{
    uint8_t pressure_data[3] = {0};

    if (SPL_ReadBytes(SPL_PRESSURE_B2, pressure_data, 3) != HAL_OK)
        return;

    SPL_data->result.pressure_raw = static_cast<int32_t>(pressure_data[0]) << 16 |
                                static_cast<int32_t>(pressure_data[1] << 8 | pressure_data[2]);
    SPL_data->result.pressure_raw = (SPL_data->result.pressure_raw & 0x00800000)
                            ? (0xFF000000 | SPL_data->result.pressure_raw) : SPL_data->result.pressure_raw;

    uint8_t temperature_data[3] = {0};

    if (SPL_ReadBytes(SPL_TEMP_B2, temperature_data, 3) != HAL_OK)
        return;

    SPL_data->result.temperature_raw = static_cast<int32_t>(temperature_data[0]) << 16 |
                                       static_cast<int32_t>(temperature_data[1]) << 8 | temperature_data[2];
    SPL_data->result.temperature_raw = (SPL_data->result.temperature_raw & 0x00800000)
                            ? (0xFF000000 | SPL_data->result.temperature_raw) :SPL_data->result.temperature_raw;

    // uint8_t data[6] = {0};
    // SPL_ReadBytes(SPL_PRESSURE_B1,data,6);
    //
    // uint32_t p_raw = ((uint32_t)data[0] << 16) |
    //                      ((uint32_t)data[1] << 8) |
    //                      ((uint32_t)data[2]);
    // SPL_data->result.pressure_raw = (int32_t)(p_raw << 8) >> 8;
    //
    // uint32_t t_raw = ((uint32_t)data[3] << 16) |
    //                      ((uint32_t)data[4] << 8) |
    //                      ((uint32_t)data[5]);
    // // if (t_raw & 0x800000)
    // // {
    // //     t_raw |= 0xFF000000;
    // // }
    // SPL_data->result.temperature_raw = (int32_t)(t_raw << 8) >> 8;
}

void SPL06::spl_calculate_result()
{
    float Traw_sc = static_cast<float>(SPL_data->result.temperature_raw) / kT;
    float Praw_sc = static_cast<float>(SPL_data->result.pressure_raw / kP);

    float c00 = static_cast<float>(SPL_data->calibration_data.pressure_calibration.c00);
    float c10 = static_cast<float>(SPL_data->calibration_data.pressure_calibration.c10);
    float c20 = SPL_data->calibration_data.pressure_calibration.c20;
    float c30 = SPL_data->calibration_data.pressure_calibration.c30;
    float c01 = SPL_data->calibration_data.pressure_calibration.c01;
    float c11 = SPL_data->calibration_data.pressure_calibration.c11;
    float c21 = SPL_data->calibration_data.pressure_calibration.c21;

    SPL_data->result.pressure = c00 +
        Praw_sc * (c10 + Praw_sc * (c20 + Praw_sc * c30))
            + Traw_sc * c01
            + Traw_sc *  Praw_sc * (c11 + Praw_sc * c21);
}

void SPL06::spl_update_result()
{
    get_pressure_temp_raw_data();
    spl_calculate_result();
}

rw_lock& SPL06::get_lock()
{
    static rw_lock lock;
    return lock;
}
