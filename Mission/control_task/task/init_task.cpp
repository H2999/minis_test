#include "MPU6050.h"
#include "SPL.h"
#include "tim.h"

extern "C"
{
    void init_task(void *argument)
    {
        //mpu6050初始化
        MPU6050::getinstance().MPU6050_Init();
        //spl06初始化
        if (SPL06::getinstance().SPL_init() == HAL_OK)
        {
            //只需在初始化时读取一次出厂校准参数
            SPL06::getinstance().get_calibration_data();
        }
        //开启pwm输出
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 50);
        //开启输入比较
        HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);

        vTaskDelete(nullptr);
    }
}
