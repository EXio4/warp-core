#pragma once
#include <memory>
#include <condition_variable>
#include <mutex>
#include <shared_mutex>
#include "readwrite_mutex.hpp"


template <typename T>
class MVar
{
public:
    MVar(void);
    MVar(const std::shared_ptr<T>& value);
    ~MVar(void);

    std::shared_ptr<T> take();
    std::shared_ptr<T> read();
    void put(const std::shared_ptr<T>& value);

private:
    std::mutex m_mutex;
    readwrite_mutex m_rw_mutex;
    std::condition_variable m_put_cond;
    std::condition_variable m_take_cond;
    std::condition_variable m_read_cond;
    std::shared_ptr<T> m_value;
};

template <typename T>
MVar<T>::MVar(void)
{
}

template <typename T>
MVar<T>::MVar(const std::shared_ptr<T>& value)
{
    put(value);
}

template <typename T>
MVar<T>::~MVar(void)
{
}

template <typename T>
std::shared_ptr<T> MVar<T>::take()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    while(!m_value) { m_take_cond.wait(lock); }
    std::lock_guard<write_mutex> w_lock(m_rw_mutex);
    auto token_value = m_value;
    m_value.reset();
    m_put_cond.notify_one();
    return token_value;
}

template <typename T>
std::shared_ptr<T> MVar<T>::read()
{
    std::lock_guard<read_mutex> r_lock(m_rw_mutex);
    std::mutex m; std::unique_lock<std::mutex> lock(m);
    while(!m_value) { m_read_cond.wait(lock); }
    return m_value;
}

template <typename T>
void MVar<T>::put(const std::shared_ptr<T>& value)
{
    if (!value) return;

    std::unique_lock<std::mutex> lock(m_mutex);
    while(m_value) { m_put_cond.wait(lock); }
    m_value = value;
    m_read_cond.notify_all();
    m_take_cond.notify_one();
}
