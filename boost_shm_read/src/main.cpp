#if 0
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/containers/list.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <string>
#include <iostream>
#include <cstdlib> //std::system
#include <thread>

using namespace boost::interprocess;

using MyMutex = interprocess_mutex;

//Define an STL compatible allocator of ints that allocates from the managed_shared_memory.
//This allocator will allow placing containers in the segment
typedef allocator<int, managed_shared_memory::segment_manager>  ShmemAllocator;

//Alias a vector that uses the previous STL-like allocator so that allocates
//its values from the segment
typedef vector<int, ShmemAllocator> MyVector;
typedef list<int, ShmemAllocator> MyList;

void Launchchildprocess(std::string &argv)
{
    std::string s = argv; s += " child ";
    //std::cout << s << std::endl;
    std::system(s.c_str());
    //if(0 != std::system(s.c_str())) std::cout << false<< std::endl;
}


//Main function. For parent process argc == 1, for child process argc == 2
int main(int argc, char *argv[])
{
   if(argc == 1){ //Parent process
      //Remove shared memory on construction and destruction
      struct shm_remove
      {
         shm_remove() { shared_memory_object::remove("MySharedMemory"); }
         ~shm_remove(){ shared_memory_object::remove("MySharedMemory"); }
      } remover;

      //Create a new segment with given name and size
      managed_shared_memory segment(create_only, "MySharedMemory", 4096000);

      //Initialize shared memory STL-compatible allocator
      const ShmemAllocator alloc_inst (segment.get_segment_manager());

      //Construct a vector named "MyVector" in shared memory with argument alloc_inst
       MyMutex *mymutex = segment.construct<MyMutex>("MyMutex")();
       //boost::interprocess::named_mutex mymutex(boost::interprocess::open_or_create, "MyMutex");
       MyList *mylist = segment.construct<MyList>("MyList")(alloc_inst);
       std::string s(argv[0]);
       std::thread myThread(Launchchildprocess, std::ref(s));
       myThread.detach();

      for(int i = 0; i < 10240; ++i)  //Insert data in the vector
      {
          std::cout << "myvector add one-->" << i << std::endl;
          //mymutex.lock();
          mymutex->lock();
          mylist->push_back(i);
          //mymutex.unlock();
          mymutex->unlock();
      }

      /*
      //Launch child process
      std::string s(argv[0]); s += " child ";
      if(0 != std::system(s.c_str()))
         return 1;
      */

      //Check child has destroyed the vector
      if(segment.find<MyVector>("MyVector").first)
         return 1;

   }
   else{ //Child process
      //Open the managed segment
      managed_shared_memory segment(open_only, "MySharedMemory");

      //Find the vector using the c-string name
      MyMutex *mymutex = segment.find<MyMutex>("MyMutex").first;
      MyList *mylist = segment.find<MyList>("MyList").first;
       int command = 0;
       for (int k = 0; k < 10240; k++) {
           bool result = false;
           mymutex->lock();
           if (!mylist->empty()) {
               command = mylist->front();
               mylist->pop_front();
               mymutex->unlock();
               result = true;
           } else {
               mymutex->unlock();
               result = false;
           }
           if (result) {
               std::cout << "show a number " << command << std::endl;
           } else {
               std::cout << "qunue is empty" << k << std::endl;
           }
           std::cout << "end" << std::endl;
       }
      /*
      //Use vector in reverse order
      std::sort(myvector->rbegin(), myvector->rend());
      for (MyVector ::iterator it=myvector->begin(); it!=myvector->end(); ++it)
           std::cout << ' ' << *it;
       std::cout << '\n';
      */
      //When done, destroy the vector from the segment
      segment.destroy<MyVector>("MyVector");

   }

   return 0;
}
#endif

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
            logInfo(boost_shm, "revice msg start");
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