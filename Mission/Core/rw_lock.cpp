#include "rw_lock.h"

rw_lock::rw_lock()
{
    read_gate = xSemaphoreCreateBinary();
    write_gate = xSemaphoreCreateBinary();
    xMutex = xSemaphoreCreateMutex();

    configASSERT(read_gate != nullptr);
    configASSERT(write_gate != nullptr);
    configASSERT(xMutex != nullptr);

    // 初始化信号量为“可用”状态
    xSemaphoreGive(read_gate);
    xSemaphoreGive(write_gate);
}

rw_lock::~rw_lock()
{
    vSemaphoreDelete(read_gate);
    vSemaphoreDelete(write_gate);
    vSemaphoreDelete(xMutex);
}

void rw_lock::write_lock()
{
    xSemaphoreTake(xMutex, portMAX_DELAY);
    _writer_waiting_count ++;
    first_writer = (_writer_waiting_count == 1);
    xSemaphoreGive(xMutex);

    if (first_writer)
    {
        xSemaphoreTake(read_gate, portMAX_DELAY);
    }
    xSemaphoreTake(write_gate, portMAX_DELAY);
}

void rw_lock::write_unlock()
{
    //这里把拿锁放在lock 释放锁放在unlock中是因为写者同一时刻只能有一个
    xSemaphoreGive(write_gate);

    xSemaphoreTake(xMutex, portMAX_DELAY);
    _writer_waiting_count --;
    xSemaphoreGive(xMutex);

    if (_writer_waiting_count == 0)
    {
        xSemaphoreGive(read_gate);
    }

}

void rw_lock::read_lock()
{
    //读者进来的时候要先获取信号量 如果不先检查是否有信号量的话那写者获取了读者大门也没用 因为读者直接无视
    xSemaphoreTake(read_gate, portMAX_DELAY);

    xSemaphoreTake(xMutex, portMAX_DELAY);
    _reader_count ++;
    first_reader = (_writer_waiting_count == 1);
    xSemaphoreGive(xMutex);

    if (first_reader)
    {
        xSemaphoreTake(write_gate, portMAX_DELAY);
    }
    xSemaphoreGive(read_gate);
}

void rw_lock::read_unlock()
{
    //xSemaphoreGive(read_gate); 不放在unlock中的原因是 如果在lock中拿锁 在unlock中放锁
    //那同一时刻只能有一个读者在读 不满足想要的多读者并发读取的目的
    //这个读者大门的唯一作用就是用来在有写者在写的时候挡住读者用的
    xSemaphoreTake(xMutex, portMAX_DELAY);
    _reader_count--;
    if (_reader_count == 0)
    {
        // 最后一个读者释放 写者大门
        xSemaphoreGive(write_gate);
    }
    xSemaphoreGive(xMutex);
}


