#ifndef RW_LOCK_H
#define RW_LOCK_H
#include "FreeRTOS.h"
#include "semphr.h"

class rw_lock
{
public:
    rw_lock();
    ~rw_lock();

    void read_lock();
    void read_unlock();
    void write_lock();
    void write_unlock();
private:
    SemaphoreHandle_t read_gate;   //读者大门
    SemaphoreHandle_t write_gate;  //写者大门

    //读者大门就是为了在有写者的时候能让写者拦住读者实现写优先目的的
    //写者大门就是为了在有读者还没读完的时候拦住写者不让它直接抢断的 只有前面的读者读完才抢断
    SemaphoreHandle_t xMutex;

    volatile int16_t _reader_count;         // 当前正在读取的读者数量
    volatile int16_t _writer_waiting_count; // 正在等待的写者数量

    bool first_writer{};
    bool first_reader{};
};

class read_scoped_lock
{
public:
    explicit read_scoped_lock(rw_lock &lock):_lock(lock),_is_locked(true)
    {
        _lock.read_lock();
    }
    ~read_scoped_lock()
    {
        if (_is_locked)
        {
            _lock.read_unlock();
        }
    }

    read_scoped_lock(const read_scoped_lock&) = delete;
    read_scoped_lock& operator=(const read_scoped_lock&) = delete;
private:
    rw_lock &_lock;
    bool _is_locked{};
};

class write_scoped_lock
{
public:
    explicit write_scoped_lock(rw_lock &lock):_lock(lock),_is_locked(true)
    {
        _lock.write_lock();
    }
    ~write_scoped_lock()
    {
        if (_is_locked)
        {
            _lock.write_unlock();
        }
    }

    write_scoped_lock(const write_scoped_lock&) = delete;
    write_scoped_lock& operator=(const write_scoped_lock&) = delete;
private:
    rw_lock &_lock;
    bool _is_locked{};
};

#endif //RW_LOCK_H