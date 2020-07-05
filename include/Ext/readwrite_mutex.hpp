#include <mutex>
#include <condition_variable>
#include <thread>

class base_mutex
{
public:
    base_mutex(): m_state(0) {}

    
// Exclusive ownership

    void lock_exclusive()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_state & m_write_entered)
        {
            m_gate1.wait(lock);
        }
        m_state |= m_write_entered;
        while (m_state & m_n_readers)
        {
            m_gate2.wait(lock);
        }
    }

    void unlock_exclusive()
    {
        {
            std::lock_guard<std::mutex> _(m_mutex);
            m_state = 0;
        }
        m_gate1.notify_all();
    }

// Shared ownership

    void lock_shared()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while ((m_state & m_write_entered) || ((m_state & m_n_readers) == m_n_readers))
        {
            m_gate1.wait(lock);
        }
        auto num_readers = (m_state & m_n_readers) + 1;
        m_state &= ~m_n_readers;
        m_state |= num_readers;
    }

    void unlock_shared()
    {
        std::lock_guard<std::mutex> _(m_mutex);
        auto num_readers = (m_state & m_n_readers) - 1;
        m_state &= ~m_n_readers;
        m_state |= num_readers;
        if (m_state & m_write_entered)
        {
            if (num_readers == 0)
            {
                m_gate2.notify_one();
            }
        }
        else
        {
            if (num_readers == m_n_readers - 1)
            {
                m_gate1.notify_one();
            }
        }
    }

private:
    std::mutex m_mutex;
    std::condition_variable m_gate1;
    std::condition_variable m_gate2;
    unsigned m_state;

    static const unsigned m_write_entered = 1U << (sizeof(unsigned)*CHAR_BIT - 1);
    static const unsigned m_n_readers = ~m_write_entered;
};

class read_mutex : public virtual base_mutex
{
public:
    void lock()
    {
        base_mutex::lock_shared();
    }
    
    void unlock()
    {
        base_mutex::unlock_shared();
    }
};

class write_mutex : public virtual base_mutex
{
public:
    void lock()
    {
        base_mutex::lock_exclusive();
    }

    void unlock()
    {
        base_mutex::unlock_exclusive();
    }
};

class readwrite_mutex : public read_mutex, public write_mutex
{
};
