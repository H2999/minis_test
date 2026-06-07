#ifndef SPL06_H
#define SPL06_H


#include "stm32f1xx_hal.h"
#include <stdint.h>

#define kP 1040384.0f
#define kT 524288.0f

typedef struct
{
    int32_t c00;
    int32_t c10;
    int16_t c20;
    int16_t c30;
    int16_t c01;
    int16_t c11;
    int16_t c21;
}pressure_t;

typedef struct
{
    int16_t c0;
    int16_t c1;
}temp_t;

typedef struct
{
    int32_t pressure_raw;
    int32_t temperature_raw;

    float pressure;
    float temperature;
}data_t;

typedef struct
{
    pressure_t pressure_calibration;
    temp_t temperature_calibration;
}Calibration_t;

typedef struct
{
    data_t result;
    Calibration_t calibration_data;
}SPL_t;

HAL_StatusTypeDef SPL_WriteReg(uint8_t reg, uint8_t data);
HAL_StatusTypeDef SPL_ReadReg(uint8_t reg, uint8_t *data);
HAL_StatusTypeDef SPL_ReadBytes(uint8_t reg, uint8_t *data,uint8_t len);
HAL_StatusTypeDef SPL_init();
void get_calibration_data(SPL_t *SPL_data);
void get_pressure_temp_raw_data(SPL_t *SPL_data);
void spl_get_result(SPL_t *SPL_data);

#endif