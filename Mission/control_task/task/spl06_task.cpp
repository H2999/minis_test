#include "FreeRTOS.h"
#include "task.h"
#include "rw_lock.h"
#include "SPL.h"
#include "tim.h"
#include "usart.h"

struct frame
{
    float data;
    uint8_t tail[4];
}spl_tx_data;

float pressure{};
float pressure0 = 100919;
float pressure50 = 100916;

float compare{};

extern "C"
{
    void update_spl_pressure_data()
    {
        //自动上读锁，防止在拷贝数据时被 spl06_thread 强行打断导致数据错乱
        write_scoped_lock spl_lock(SPL06::get_lock());
        SPL06::getinstance().spl_update_result();
    }

    float get_spl_pressure_data()
    {
        read_scoped_lock spl_lock(SPL06::get_lock());
        return SPL06::getinstance().get_pressure();
    }

    void spl06_thread(void *argument)
    {
        spl_tx_data.tail[0] = 0x00;
        spl_tx_data.tail[1] = 0x00;
        spl_tx_data.tail[2] = 0x80;
        spl_tx_data.tail[3] = 0x7f;

        for (int i = 0; i < 200; i++)
        {
            update_spl_pressure_data();
            vTaskDelay(20);
        }

        while (1)
        {
            update_spl_pressure_data();
            pressure = get_spl_pressure_data();

            spl_tx_data.data = get_spl_pressure_data();

            HAL_UART_Transmit_DMA(&huart2,reinterpret_cast<uint8_t *>(&spl_tx_data),sizeof(spl_tx_data));

            // 计算亮度百分比：初始位置(压力高)最亮，升高后(压力低)熄灭
            float brightness;
            if (pressure >= pressure0)
            {
                brightness = 0.0f;      // 低于初始高度，最亮
            }
            else if (pressure <= pressure50)
            {
                brightness = 1.0f;      // 超过50cm，熄灭
            }
            else
            {
                brightness = (pressure - pressure50) / (pressure0 - pressure50);
            }

            compare = brightness * 100.0f;

            // 边界保护
            if (compare > 100) compare = 100;
            if (compare < 0) compare = 0;

            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, compare);

            vTaskDelay(10);
        }
    }

    void start_spl06_task(void * argument)
    {
        xTaskCreate(spl06_thread,"spl06_thread",256,nullptr,
            configMAX_PRIORITIES - 3, nullptr);
        vTaskDelete(nullptr);
    }

}
