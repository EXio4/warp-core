#pragma once
#include <memory>
#include <condition_variable>
#include <mutex>
#include <shared_mutex>


template <typename T>
class MVar
{
public:
    MVar(void);
    ~MVar(void);

    std::unique_ptr<T> read();
    void write(std::unique_ptr<T> value);

private:
    std::mutex m_mutex;
    bool empty;
    std::condition_variable m_empty_cond;
    std::condition_variable m_full_cond;
    std::unique_ptr<T> m_value;
};

template <typename T>
MVar<T>::MVar(void)
{
}


template <typename T>
MVar<T>::~MVar(void) {
}


template <typename T>
std::unique_ptr<T> MVar<T>::read()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_full_cond.wait(lock, [&]() { return m_value.get() != nullptr; });
    std::unique_ptr ret = std::move(m_value);
    m_empty_cond.notify_one();
    return ret;
}

template <typename T>
void MVar<T>::write(std::unique_ptr<T> value)
{
    if (!value) return;

    std::unique_lock<std::mutex> lock(m_mutex);
    m_empty_cond.wait(lock, [&]() { return m_value.get() == nullptr; });
    m_value = std::move(value);
    m_full_cond.notify_one();
}
