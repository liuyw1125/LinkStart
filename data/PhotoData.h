#ifndef PHOTODATA_H
#define PHOTODATA_H

template <class T>
class DataItem
{
public:
    DataItem(){}
    ~DataItem(){}
    void mutexLock()
    {
        m_mutex.lock();
    }
    void mutexUnlock()
    {
        m_mutex.unlock();
    }
    T data;
public:
    std::mutex m_mutex;
};

#endif
