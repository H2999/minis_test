#include <cstring>

#include "stm32f1xx.h"
#include "stm32f1xx_hal_tim.h"
#include "FreeRTOS.h"
#include "task.h"
#include "usart.h"

struct frame_head
{
    uint8_t head = 0xA5;
};

struct rx_data_t
{
    frame_head head;
    uint8_t data[30]{};
    uint8_t data_len;
};

uint8_t rx_buffer[2][50];
rx_data_t pwm_rx_data;
uint8_t rx_index = 0;

volatile uint8_t data_received_flag = 0;

uint16_t target_frequency;
uint8_t target_duty;

struct frame
{
    float data;
    uint8_t tail[4];
}pwm_tx_data;

extern "C"
{
    void parse_data(uint8_t* data, uint8_t size)
    {
        uint16_t freq = 0;
        uint8_t duty = 0;
        uint8_t step = 0;  // 0=解析频率, 1=解析占空比
        uint8_t i = 0;

        for (i = 0; i < size; i++)
        {
            char c = data[i];

            // 如果是数字
            if (c >= '0' && c <= '9')
            {
                if (step == 0)
                {
                    // 解析频率
                    freq = freq * 10 + (c - '0');
                }
                else
                {
                    // 解析占空比
                    duty = duty * 10 + (c - '0');
                }
            }
            // 如果是空格，切换到解析占空比
            else if (c == ' ')
            {
                step = 1;
            }
        }

        target_frequency = freq;
        target_duty = duty;
    }

    void pwm_task(void *argument)
    {
        pwm_tx_data.tail[0] = 0x00;
        pwm_tx_data.tail[1] = 0x00;
        pwm_tx_data.tail[2] = 0x80;
        pwm_tx_data.tail[3] = 0x7f;

        HAL_UARTEx_ReceiveToIdle_DMA(&huart2,rx_buffer[0],sizeof(rx_buffer[0]));
        while (1)
        {
            if (data_received_flag)
            {
                data_received_flag = 0;
                parse_data(pwm_rx_data.data,pwm_rx_data.data_len);

            }
            pwm_tx_data.data = target_duty;
            HAL_UART_Transmit_DMA(&huart2,reinterpret_cast<uint8_t *>(&pwm_tx_data),sizeof(pwm_tx_data));

            vTaskDelay(10);
        }
    }

    // void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
    // {
    //     if (htim->Instance == TIM3 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    //     {
    //         uint32_t capture_value = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
    //
    //     }
    // }

    void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
    {
        if (huart == &huart2)
        {
            // 获取当前缓冲区指针
            uint8_t* current_buffer = rx_buffer[rx_index];

            // 检查帧头
            if (Size >= 1 && current_buffer[0] == 0xA5)
            {

                // 计算实际数据长度（最多30字节）
                uint16_t data_len = (Size - 1) > 30 ? 30 : (Size - 1);
                pwm_rx_data.data_len = data_len;
                // 复制数据到 rx_data 结构体
                pwm_rx_data.head.head = current_buffer[0];
                memcpy(pwm_rx_data.data, current_buffer + 1, data_len);
                data_received_flag = 1;
            }

            // 切换到另一个缓冲区
            rx_index ^= 0x01;

            // 重新启动 DMA 接收新数据
            HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rx_buffer[rx_index], sizeof(rx_buffer[rx_index]));
        }
    }
}
