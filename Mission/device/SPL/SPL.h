#ifndef SPL06_H
#define SPL06_H

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include "spl_reg.h"
#include "rw_lock.h"

class SPL06
{
public:
    ~SPL06();
    static SPL06 &getinstance();

    SPL06(const SPL06 &) = delete;
    SPL06 & operator=(const SPL06 &) = delete;

    HAL_StatusTypeDef SPL_init();
    void get_calibration_data();
    void spl_update_result();

    float get_pressure() const
    {
        return SPL_data->result.pressure;
    }

    [[nodiscard]] static rw_lock &get_lock();
private:
    SPL06();
    static HAL_StatusTypeDef SPL_WriteReg(uint8_t reg, uint8_t data);
    static HAL_StatusTypeDef SPL_ReadReg(uint8_t reg, uint8_t *data);
    static HAL_StatusTypeDef SPL_ReadBytes(uint8_t reg, uint8_t *data,uint8_t len);

    void get_pressure_temp_raw_data();
    void spl_calculate_result();

    struct pressure_t
    {
        int32_t c00;
        int32_t c10;
        int16_t c20;
        int16_t c30;
        int16_t c01;
        int16_t c11;
        int16_t c21;
    };

    struct temp_t
    {
        int16_t c0;
        int16_t c1;
    };

    struct data_t
    {
        int32_t pressure_raw;
        int32_t temperature_raw;

        float pressure;
        float temperature;
    };

    struct Calibration_t
    {
        pressure_t pressure_calibration;
        temp_t temperature_calibration;
    };

    struct SPL_t
    {
        data_t result;
        Calibration_t calibration_data;
    };

    SPL_t* SPL_data = nullptr;

    float pressure_buffer[200]{};

    static constexpr float kP = 1040384.0f;
    static constexpr float kT = 524288.0f;
};

#endif