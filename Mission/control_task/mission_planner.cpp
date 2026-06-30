#include "FreeRTOS.h"
#include "task.h"

extern "C"
{
    extern void start_mpu6050_task(void *argument);
    extern void start_spl06_task(void *argument);
    extern void init_task(void *argument);
    extern void pwm_task(void *argument);
    extern void attitude_task(void *argument);

    void task_control(void const * argument)
    {
        xTaskCreate(init_task,"init_task",128,nullptr,
                    configMAX_PRIORITIES - 1, nullptr);
        // xTaskCreate(start_mpu6050_task,"mpu",128,nullptr,
        //             configMAX_PRIORITIES - 2, nullptr);
        // xTaskCreate(pwm_task,"pwm",512,nullptr,
        //             configMAX_PRIORITIES - 2, nullptr);
        // xTaskCreate(start_spl06_task,"spl",128,nullptr,
        //             configMAX_PRIORITIES - 2, nullptr);
        xTaskCreate(attitude_task,"attitude",256,nullptr,
                    configMAX_PRIORITIES - 2, nullptr);

        vTaskDelete(nullptr);
    }
}
