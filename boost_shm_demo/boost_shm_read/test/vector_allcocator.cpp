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