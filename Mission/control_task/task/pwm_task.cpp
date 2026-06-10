// #include <cstring>
//
// #include "stm32f1xx.h"
// #include "stm32f1xx_hal_tim.h"
// #include "FreeRTOS.h"
// #include "task.h"
// #include "tim.h"
// #include "usart.h"
// #include <stdio.h>
// #include <string.h>
//
// //接收数据的帧头
// struct frame_head
// {
//     uint8_t head = 0xA5;
// };
// //接收数据的结构体
// struct rx_data_t
// {
//     frame_head head;
//     uint8_t data[30]{};
//     uint8_t data_len{};
// };
// //用于串口示波的结构体
// struct frame
// {
//     float data;
//     uint8_t tail[4];
// }pwm_tx_data;
// //双缓冲区接收数据
// uint8_t rx_buffer[2][50];
// rx_data_t pwm_rx_data;
// uint8_t rx_index = 0;
// //接收完成标志位
// volatile uint8_t data_received_flag = 0;
// //解析接收到的数据
// uint16_t target_frequency = 1000; // 默认 1kHz
// uint8_t target_duty = 50;         // 默认 50%
// uint16_t prescaler;
//
// // ==================== 输入捕获相关（单通道测频+测占空比）====================
// // 状态机
// enum class capture_state
// {
//     WAIT_RISING_EDGE,    // 等待第一个上升沿
//     WAIT_FALLING_EDGE,   // 等待下降沿（测量高电平）
//     WAIT_NEXT_RISING     // 等待下一个上升沿（测量周期）
// };
//
// volatile capture_state cap_state = capture_state::WAIT_RISING_EDGE;
// volatile uint32_t rise_time = 0;      // 上升沿时间
// volatile uint32_t fall_time = 0;      // 下降沿时间
// volatile uint32_t period_ticks = 0;    // 周期（计数值）
// volatile uint32_t high_time_ticks = 0; // 高电平时间（计数值）
// volatile uint8_t capture_ready = 0;    // 一次完整测量完成标志
//
// // 测量结果（供任务读取）
// volatile float measured_frequency = 0;
// volatile float measured_duty_cycle = 0;
//
// char buffer[64];
//
// // 定时器时钟频率（Prescaler=71，1MHz计数）
// #define TIMER_CLOCK_HZ 1000000  // 1MHz = 1us/计数
//
// extern "C"
// {
//     void parse_data(const uint8_t* data, const uint8_t size)
//     {
//         uint16_t freq = 0;
//         uint8_t duty = 0;
//         uint8_t step = 0;  // 0=解析频率, 1=解析占空比
//         uint8_t i = 0;
//
//         for (i = 0; i < size; i++)
//         {
//             char c = data[i];
//
//             // 如果是数字
//             if (c >= '0' && c <= '9')
//             {
//                 if (step == 0)
//                 {
//                     // 解析频率
//                     freq = freq * 10 + (c - '0');
//                 }
//                 else
//                 {
//                     // 解析占空比
//                     duty = duty * 10 + (c - '0');
//                 }
//             }
//             // 如果是空格，切换到解析占空比
//             else if (c == ' ')
//             {
//                 step = 1;
//             }
//         }
//
//         if (freq >= 1000 && freq <= 10000)  // 频率范围限制
//         {
//             target_frequency = freq;
//         }
//         if (duty <= 100)  // 占空比范围限制
//         {
//             target_duty = duty;
//         }
//     }
//
//     void pwm_task(void *argument)
//     {
//         //vofa示波用
//         pwm_tx_data.tail[0] = 0x00;
//         pwm_tx_data.tail[1] = 0x00;
//         pwm_tx_data.tail[2] = 0x80;
//         pwm_tx_data.tail[3] = 0x7f;
//
//         HAL_UARTEx_ReceiveToIdle_DMA(&huart2,rx_buffer[0],sizeof(rx_buffer[0]));
//         while (1)
//         {
//             if (data_received_flag)
//             {
//                 data_received_flag = 0;
//                 parse_data(pwm_rx_data.data,pwm_rx_data.data_len);
//             }
//
//             prescaler = (72000000  / (100 * target_frequency)) - 1;
//             __HAL_TIM_SET_PRESCALER(&htim1,prescaler);
//             __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 100);
//
//             if (capture_ready)
//             {
//                 capture_ready = 0;
//
//                 // 格式化测量结果
//                 // snprintf(buffer, sizeof(buffer),
//                 //          "Freq: %.1f Hz, Duty: %.1f%%\r\n",
//                 //          measured_frequency, measured_duty_cycle);
//
//                 // 通过串口发送
//                 HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
//             }
//
//             //串口示波用
//             // pwm_tx_data.data = target_duty;
//             // HAL_UART_Transmit_DMA(&huart2,reinterpret_cast<uint8_t *>(&pwm_tx_data),sizeof(pwm_tx_data));
//
//             vTaskDelay(10);
//         }
//     }
//
//     void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
// {
//     if (htim->Instance == TIM3 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
//     {
//         uint32_t capture_value = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
//
//         switch(cap_state)
//         {
//         case capture_state::WAIT_RISING_EDGE:
//             rise_time = capture_value;
//             // 改为下降沿捕获
//             __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_FALLING);
//             cap_state = capture_state::WAIT_FALLING_EDGE;
//             break;
//
//         case capture_state::WAIT_FALLING_EDGE:
//             fall_time = capture_value;
//             // 计算高电平时间（考虑溢出）
//             if (fall_time >= rise_time)
//                 high_time_ticks = fall_time - rise_time;
//             else
//                 high_time_ticks = (0xFFFF - rise_time) + fall_time + 1;
//
//             // 改为上升沿捕获
//             __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
//             cap_state = capture_state::WAIT_NEXT_RISING;
//             break;
//
//         case capture_state::WAIT_NEXT_RISING:
//             {
//                 uint32_t current_rise = capture_value;
//                 // 计算周期
//                 if (current_rise >= rise_time)
//                     period_ticks = current_rise - rise_time;
//                 else
//                     period_ticks = (0xFFFF - rise_time) + current_rise + 1;
//
//                 if (period_ticks > 0)
//                 {
//                     measured_frequency = 1000000.0f / static_cast<float>(period_ticks);
//                     measured_duty_cycle = static_cast<float>(high_time_ticks) / static_cast<float>(period_ticks) * 100.0f;
//                     capture_ready = 1;
//                 }
//
//                 // 关键修正：准备下一次测量
//                 rise_time = current_rise;  // 保存当前上升沿作为下一个周期的起点
//                 cap_state = capture_state::WAIT_FALLING_EDGE;  // 改为等待下降沿
//                 __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_FALLING);
//                 break;
//             }
//         }
//     }
// }
//
//
//
//     void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
//     {
//         if (huart == &huart2)
//         {
//             // 获取当前缓冲区指针
//             uint8_t* current_buffer = rx_buffer[rx_index];
//
//             // 检查帧头
//             if (Size >= 1 && current_buffer[0] == 0xA5)
//             {
//
//                 // 计算实际数据长度（最多30字节）
//                 uint16_t data_len = (Size - 1) > 30 ? 30 : (Size - 1);
//                 pwm_rx_data.data_len = data_len;
//                 // 复制数据到 rx_data 结构体
//                 pwm_rx_data.head.head = current_buffer[0];
//                 memcpy(pwm_rx_data.data, current_buffer + 1, data_len);
//                 data_received_flag = 1;
//             }
//
//             // 切换到另一个缓冲区
//             rx_index ^= 0x01;
//
//             // 重新启动 DMA 接收新数据
//             HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rx_buffer[rx_index], sizeof(rx_buffer[rx_index]));
//         }
//     }
// }

#include <cstring>
#include "stm32f1xx.h"
#include "stm32f1xx_hal_tim.h"
#include "FreeRTOS.h"
#include "task.h"
#include "tim.h"
#include "usart.h"
#include <cstdio>
#include <cstring>

// 接收数据的结构体
struct rx_data_t
{
    uint8_t data[32]{};
    uint8_t data_len{};
};

// 双缓冲区接收数据
uint8_t rx_buffer[2][32];
rx_data_t pwm_rx_data;
uint8_t rx_index = 0;
volatile uint8_t data_received_flag = 0;

// PWM参数
uint16_t target_frequency = 1000;
uint8_t target_duty = 50;

// 输入捕获状态机
enum capture_state
{
    WAIT_RISING,
    WAIT_FALLING,
    WAIT_NEXT_RISING
};

volatile enum capture_state cap_state = WAIT_RISING;
volatile uint32_t rise_time = 0;
volatile uint32_t fall_time = 0;
volatile uint32_t period_ticks = 0;
volatile uint32_t high_ticks = 0;
volatile uint8_t capture_ready = 0;
volatile float measured_freq = 0;
volatile float measured_duty = 0;

char buffer[64];

// 更新PWM输出
void update_pwm(uint16_t freq, uint8_t duty)
{
    if(freq < 1000 || freq > 10000) return;
    if(duty > 100) return;

    uint32_t period = 1000000 / freq;  // 1MHz计数频率

    if(period >= 1 && period <= 65535)
    {
        // 停止PWM
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);

        // 更新参数
        __HAL_TIM_SET_AUTORELOAD(&htim1, period - 1);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, (period * duty) / 100);

        // 重新启动PWM
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    }
}

// 解析串口数据（直接接收数字，不需要帧头）
void parse_data(const uint8_t* data, uint8_t size)
{
    uint16_t freq = 0;
    uint8_t duty = 0;
    uint8_t step = 0;

    for(uint8_t i = 0; i < size; i++)
    {
        char c = data[i];

        if(c >= '0' && c <= '9')
        {
            if(step == 0)
                freq = freq * 10 + (c - '0');
            else
                duty = duty * 10 + (c - '0');
        }
        else if(c == ' ' || c == ',')
        {
            step = 1;
        }
    }

    if(freq >= 1000 && freq <= 10000)
        target_frequency = freq;
    if(duty <= 100)
        target_duty = duty;

    // 立即更新PWM
    update_pwm(target_frequency, target_duty);
}

extern "C"
{
    // 输入捕获中断回调
    void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
    {
        if(htim->Instance == TIM3)
        {
            uint32_t capture_value = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

            switch(cap_state)
            {
            case WAIT_RISING:
                // 第一次上升沿
                rise_time = capture_value;
                // 改为下降沿捕获
                TIM3->CCER |= TIM_CCER_CC1P;  // 下降沿
                cap_state = WAIT_FALLING;
                break;

            case WAIT_FALLING:
                // 下降沿
                fall_time = capture_value;
                // 计算高电平时间
                if(fall_time >= rise_time)
                    high_ticks = fall_time - rise_time;
                else
                    high_ticks = (0xFFFF - rise_time) + fall_time + 1;

                // 改为上升沿捕获
                TIM3->CCER &= ~TIM_CCER_CC1P;  // 上升沿
                cap_state = WAIT_NEXT_RISING;
                break;

            case WAIT_NEXT_RISING:
                // 下一个上升沿
                uint32_t next_rise = capture_value;
                // 计算周期
                if(next_rise >= rise_time)
                    period_ticks = next_rise - rise_time;
                else
                    period_ticks = (0xFFFF - rise_time) + next_rise + 1;

                if(period_ticks > 0)
                {
                    measured_freq = 1000000.0f / period_ticks;
                    measured_duty = (float)high_ticks / period_ticks * 100.0f;
                    capture_ready = 1;
                }

                // 准备下一次测量
                rise_time = next_rise;
                // 改为下降沿捕获
                TIM3->CCER |= TIM_CCER_CC1P;
                cap_state = WAIT_FALLING;
                break;
            }

            // 清除中断标志
            TIM3->SR &= ~TIM_SR_CC1IF;
        }
    }

    // 串口空闲中断回调（DMA接收）
    void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
    {
        if(huart == &huart2)
        {
            if(Size > 0)
            {
                uint8_t* current_buffer = rx_buffer[rx_index];
                pwm_rx_data.data_len = (Size > 31) ? 31 : Size;
                memcpy(pwm_rx_data.data, current_buffer, pwm_rx_data.data_len);
                data_received_flag = 1;
            }

            // 切换缓冲区
            rx_index ^= 0x01;

            // 重新启动DMA接收
            HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rx_buffer[rx_index], sizeof(rx_buffer[rx_index]));
        }
    }

    // PWM任务
    void pwm_task(void *argument)
    {
        // ========== 初始化TIM3输入捕获 ==========
        // 使能TIM3时钟
        __HAL_RCC_TIM3_CLK_ENABLE();

        // 配置TIM3基础参数
        TIM3->PSC = 71;          // 72MHz/72 = 1MHz
        TIM3->ARR = 0xFFFF;      // 最大自动重载值
        TIM3->CNT = 0;           // 清零计数器
        TIM3->CR1 = TIM_CR1_CEN; // 使能计数器

        // 配置通道1为输入捕获
        TIM3->CCMR1 = TIM_CCMR1_CC1S_0;  // CC1通道配置为输入，IC1映射到TI1
        TIM3->CCER = TIM_CCER_CC1E;      // 使能捕获，上升沿
        TIM3->DIER = TIM_DIER_CC1IE;     // 使能捕获中断

        // 配置NVIC
        NVIC_SetPriority(TIM3_IRQn, 1);
        NVIC_EnableIRQ(TIM3_IRQn);

        // ========== 初始化PWM输出 ==========
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
        __HAL_TIM_SET_PRESCALER(&htim1, 71);  // 1MHz
        update_pwm(1000, 50);

        // ========== 初始化串口DMA接收 ==========
        HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rx_buffer[0], sizeof(rx_buffer[0]));

        // 发送启动消息
        sprintf(buffer, "PWM Task Started!\r\n");
        HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);

        uint32_t last_send = 0;

        while(1)
        {
            // 处理接收到的串口数据
            if(data_received_flag)
            {
                data_received_flag = 0;
                parse_data(pwm_rx_data.data, pwm_rx_data.data_len);

                // 发送确认消息
                sprintf(buffer, "Set: %dHz, %d%%\r\n", target_frequency, target_duty);
                HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
            }

            // 每秒发送一次测量结果
            uint32_t now = xTaskGetTickCount();
            if((now - last_send) >= 1000)
            {
                last_send = now;

                if(capture_ready)
                {
                    capture_ready = 0;
                    // 将浮点数转换为整数再打印
                    const int freq_int = static_cast<int>(measured_freq + 0.5f);
                    const int duty_int = static_cast<int>(measured_duty + 0.5f);

                    sprintf(buffer, "Frequency: %d, Duty Ratio: %d%%\r\n", freq_int, duty_int);
                    HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
                }
                else
                {
                    // 如果还没有测量到数据
                    sprintf(buffer, "Waiting for signal...\r\n");
                    HAL_UART_Transmit(&huart2, reinterpret_cast<uint8_t*>(buffer), strlen(buffer), 100);
                }
            }

            vTaskDelay(10);
        }
    }
}
