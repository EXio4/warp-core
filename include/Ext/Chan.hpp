#include <memory>
#include "MVar.hpp"

template <typename T>
struct Item;

template <typename T>
struct Stream
{
    typedef MVar<Item<T>> type;
};

template <typename T>
struct Item
{
    typedef typename Stream<T>::type StreamType;
    typedef std::shared_ptr<StreamType> StreamPtr;

    T m_v;
    StreamPtr m_tail;

    Item(const T& v, const StreamPtr& tail): m_v(v), m_tail(tail) {}
};

template <typename T>
class Chan
{
private:

    typedef typename Stream<T>::type    StreamType;
    typedef MVar<StreamType>            StreamMVar;
    typedef std::shared_ptr<StreamMVar> StreamMVarPtr;

    StreamMVarPtr m_readEnd;
    StreamMVarPtr m_writeEnd;


public:

    Chan()
    {
        m_readEnd  = std::make_shared<StreamMVar>();
        m_writeEnd = std::make_shared<StreamMVar>();
        auto hole  = std::make_shared<StreamType>();

        m_readEnd->put(hole);
        m_writeEnd->put(hole);
    }

    Chan(const StreamMVarPtr& readEnd, const StreamMVarPtr& writeEnd)
        : m_readEnd(readEnd), m_writeEnd(writeEnd) {}

    T read()
    {
        auto stream = m_readEnd->take();
        auto item = stream->read();
        m_readEnd->put(item->m_tail);
        return item->m_v;
    }

    void write(const T& v)
    {
        auto newHole = std::make_shared<StreamType>();
        auto oldHole = m_writeEnd->take();
        oldHole->put(std::make_shared<Item<T>>(v, newHole));
        m_writeEnd->put(newHole);
    }

    Chan<T> dup()
    {
        auto hole = m_writeEnd->read();
        auto newReadEnd = std::make_shared<StreamMVar>(hole);
        return Chan(newReadEnd, m_writeEnd);
    }
};
