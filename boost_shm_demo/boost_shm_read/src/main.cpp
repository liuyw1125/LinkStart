#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <iostream>
#include <cstring>
#include <boost/core/no_exceptions_support.hpp>             //BOOST_TRY

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include "log/Log.hpp"


struct trace_queue
{
    enum { LineSize = 100 };

    trace_queue()
            :  message_in(false)
    {}

    //Mutex to protect access to the queue
    boost::interprocess::interprocess_mutex      mutex;

    //Condition to wait when the queue is empty
    boost::interprocess::interprocess_condition  cond_empty;

    //Condition to wait when the queue is full
    boost::interprocess::interprocess_condition  cond_full;

    //Items to fill
    char   items[LineSize];

    //Is there any message
    bool message_in;
};

using namespace boost::interprocess;
using namespace  eprosima::fastdds::dds;

int main ()
{
    //Create a shared memory object.
    shared_memory_object shm
            (open_only                    //only create
                    ,"MySharedMemory"              //name
                    ,read_write                   //read-write mode
            );

    BOOST_TRY{
            //Map the whole shared memory in this process
            mapped_region region
                    (shm                       //What to map
                            ,read_write //Map it as read-write
                    );

            //Get the address of the mapped region
            void * addr       = region.get_address();

            //Obtain a pointer to the shared structure
            trace_queue * data = static_cast<trace_queue*>(addr);

            //Print messages until the other process marks the end
            bool end_loop = false;
            Log::SetVerbosity(Log::Info);
            //logInfo(boost_shm, "revice msg start");
            do{
                scoped_lock<interprocess_mutex> lock(data->mutex);
                if(!data->message_in){
                    data->cond_empty.wait(lock);
                }
                if(std::strcmp(data->items, "last message") == 0){
                    //std::cout << "read --->"<<data->items << std::endl;
                    logInfo("read -->", data->items);
                    end_loop = true;
                }
                else{
                    //Print the message
                    //std::cout << "read --->"<<data->items << std::endl;
                    logInfo("boost_shm_read", data->items);
                    //Notify the other process that the buffer is empty
                    data->message_in = false;
                    data->cond_full.notify_one();
                }
            }
            while(!end_loop);
            logInfo(boost_shm, "revice msg end");
        }
        BOOST_CATCH(interprocess_exception &ex){
            std::cout << ex.what() << std::endl;
            return 1;
        } BOOST_CATCH_END

    return 0;
}