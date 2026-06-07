//
// Created by 1 on 2026/4/6.
//

#include <stdint.h>


//初始化就执行mode1
uint8_t current_mode = 0;
uint8_t next_mode = 1;

uint8_t key = 1;

void global_init()
{
    //所有的硬件初始化
}

void global_loop()
{
    if (key)
    {
        key ++;
        if (key > 3)
        {
            key = 1;
        }
    }
}
void global_tick()
{

}

void Mode1_init()
{
    ;//硬件初始化或者其他的初始化
}

void Mode1_loop(void)
{
    // if (key)
    // {
    //     next_mode = 2;
    //     key = 0;
    // }
    //按键2检测电点灯
}

void Mode1_exit(void)
{
    //灭灯
}



void Mode2_init()
{
    //初始化
}

void Mode2_loop(void)
{
    // if (key)
    // {
    //     next_mode = 3;
    //     key = 0;
    // }
    //按键按一次自增一次
}

void Mode2_exit(void)
{
    //数值清零
}



void Mode3_init(void)
{
    //初始化
}

void Mode3_loop(void)
{
    // if (key)
    // {
    //     next_mode = 1;
    //     key = 0;
    // }
    //检测按键2 执行标志位 = ！的操作 实现按一下按键反转一次电平的状态
}

void Mode3_exit(void)
{
    //清零计数
    //失能标志位
}

void Mode3_tick(void)
{
    //在定时器溢出中断中执行这个函数 使变量增加 如果嫌1ms太快也可以再自己套一层函数等变量加到1000执行一次 这样手动分频
}


void check(void)
{
    while (1)
    {
        if (current_mode == next_mode)
        {
            switch (current_mode)
            {
                case 1:Mode1_loop();break;
                case 2:Mode2_loop();break;
                case 3:Mode3_loop();break;
            }
        }
        else
        {
            switch (current_mode)
            {
                case 1:Mode1_exit();break;
                case 2:Mode2_exit();break;
                case 3:Mode3_exit();break;
                default: ;
            }

            current_mode = next_mode;

            switch (next_mode)
            {
                case 1:Mode1_init();break;
                case 2:Mode2_init();break;
                case 3:Mode3_init();break;
                default: ;
            }
        }
    }
}

// void EXTI9_5_IRQHandler(void)
// {
//     key += 1;
// }