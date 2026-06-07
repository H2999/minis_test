#include "MPU6050.h"

extern "C"
{
    void init_task(void *argument)
    {
        MPU6050::getinstance().MPU6050_Init();

        vTaskDelete(nullptr);
    }
}
