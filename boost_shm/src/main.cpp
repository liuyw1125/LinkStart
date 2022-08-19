
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <iostream>
#include <cstdio>


#include <boost/interprocess/detail/config_begin.hpp>
//[doc_anonymous_condition_shared_data
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/core/no_exceptions_support.hpp>             //BOOST_TRY

#include "Log.hpp"

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
//]
#include <boost/interprocess/detail/config_end.hpp>



using namespace boost::interprocess;
using namespace  eprosima::fastdds::dds;

int main ()
{

    //Erase previous shared memory and schedule erasure on exit
    struct shm_remove
    {
        shm_remove() { shared_memory_object::remove("MySharedMemory"); }
        ~shm_remove(){ shared_memory_object::remove("MySharedMemory"); }
    } remover;

    //Create a shared memory object.
    shared_memory_object shm
            (create_only               //only create
                    ,"MySharedMemory"           //name
                    ,read_write                //read-write mode
            );
    BOOST_TRY{
            //Set size
            shm.truncate(sizeof(trace_queue));

            //Map the whole shared memory in this process
            mapped_region region
            (shm                       //What to map
            ,read_write //Map it as read-write
            );

            //Get the address of the mapped region
            void * addr       = region.get_address();

            //Construct the shared structure in memory
            trace_queue * data = new (addr) trace_queue;


            const int NumMsg = 100;
            Log::SetVerbosity(Log::Info);
            logInfo(boost_shm, "send msg start");
            for(int i = 0; i < NumMsg; ++i){
                scoped_lock<interprocess_mutex> lock(data->mutex);
                if(data->message_in){
                    data->cond_full.wait(lock);
                }
                if(i == (NumMsg-1)) {
                    std::sprintf(data->items, "%s", "last message");
                    std::printf("send--->%s\n", "last message");

                }
                else {
                    std::sprintf(data->items, "%s_%d", "my_trace", i);
                    //std::printf("send--->%s_%d\n", "my_trace", i);
                    //std::string s = "my_trace" + std::to_string(i);
                    logInfo(boost_shm, data->items);
                }

                //Notify to the other process that there is a message
                data->cond_empty.notify_one();

                //Mark message buffer as full
                data->message_in = true;
            }
            logInfo(boost_shm, "send msg end");
    }
    BOOST_CATCH(interprocess_exception &ex){
        std::cout << ex.what() << std::endl;
        return 1;
    } BOOST_CATCH_END

    return 0;
}