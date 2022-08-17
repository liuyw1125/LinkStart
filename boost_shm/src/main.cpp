#include <boost/interprocess/managed_shared_memory.hpp>
#include <mutex>
#include <iostream>

int main()
{
    boost::interprocess::shared_memory_object::remove("shm");
    boost::interprocess::managed_shared_memory managed_shm(boost::interprocess::open_or_create, "shm", 100000);
    int *i = managed_shm.find_or_construct<int>("Integer")();
    std::size_t size;
    std::mutex *mymutex = managed_shm.find_or_construct<std::mutex>("mtx")();
    mymutex->lock();
    ++(*i);
    std::cout << *i << std::endl;
    std::cout << i << std::endl;
    std::cout << mymutex << std::endl;
    std::cout << managed_shm.get_size() << std::endl;
    mymutex->unlock();
}