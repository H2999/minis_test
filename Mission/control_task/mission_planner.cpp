#include "FreeRTOS.h"
#include "task.h"

extern "C"
{
    extern void start_mpu6050_task(void *argument);
    extern void init_task(void *argument);

    void task_control(void const * argument)
    {
        xTaskCreate(init_task,"init_task",128,nullptr,
                    configMAX_PRIORITIES - 1, nullptr);
        xTaskCreate(start_mpu6050_task,"mpu",128,nullptr,
                    configMAX_PRIORITIES - 2, nullptr);

        vTaskDelete(nullptr);
    }
}