//test writes_shm.cpp
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main()
{
    // 1.Create shared memory
    int shmid = shmget(100, 4096, IPC_CREAT|0664);
    // 2.associated with the current process
    void * ptr =  shmat(shmid, NULL, 0);
    const char* pt = "This is a piece of shared memory";
    memcpy(ptr, pt, strlen(pt)+1);
    printf("Press any key to continue\n");
    getchar();
    // 4.Disassociate
    shmdt(ptr);
    // 5.delete shared memory
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}

#if 0
#include <iostream>

int main() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
#endif
