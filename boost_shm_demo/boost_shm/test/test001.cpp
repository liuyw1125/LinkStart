#include <iostream>
#include <boost/interprocess/shared_memory_object.hpp>

int main()
{
    std::cout << "Hello, World!" << std::endl;
    boost::interprocess::shared_memory_object shdmem(boost::interprocess::open_or_create, "Highscore", boost::interprocess::read_write);
    shdmem.truncate(1024);
    std::cout << shdmem.get_name() << std::endl;
    boost::interprocess::offset_t size;
    if (shdmem.get_size(size))
    {
        std::cout << size << std::endl;
    }

    return 0;
}

