#include <sys/mman.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>

typedef struct{ // 定义进程共享对象
    pthread_mutex_t pmutex;// 进程互斥锁
    unsigned char data[20];// 数据域
}sharedmemory;

#define SHAREDMEMORY "shm_file"// 共享文件
int main()
{
    int fd;
    fd = shm_open(SHAREDMEMORY, O_RDWR|O_CREAT, 0777);
    ftruncate(fd, 1024);
    sharedmemory *ptr = (sharedmemory *)mmap(NULL, 1024, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    memset(ptr,0, sizeof(sharedmemory));
    close(fd);// 关掉描述符，并不影响读写ptr
    if (ptr == MAP_FAILED){
        printf("open shared failed \n");
        exit(0);
    }
    pthread_mutexattr_t  mutexattr;
    pthread_mutexattr_init(&mutexattr);//初始化 mutexattr属性
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_NORMAL);
#ifdef _POSIX_THREAD_PROCESS_SHARED
    pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED);//修改属性为进程间共享
#else
# error this implementation does not support _POSIX_THREAD_PROCESS_SHARED
#endif
    pthread_mutex_init(&ptr->pmutex,&mutexattr);//初始化一把 mutex 锁
    pthread_mutexattr_destroy(&mutexattr);  //销毁 mutex 属性对象

    pthread_mutex_lock(&ptr->pmutex);
    for (unsigned char i = 0; i < 10; i++)
    {
        ptr->data[i] = i;
        printf("%d ", ptr->data[i]);
    }
    pthread_mutex_unlock(&ptr->pmutex);
    printf("end\n");
    getchar();
    pthread_mutex_destroy(&ptr->pmutex);//销毁 mutex 锁
    munmap(ptr, 1024);
    shm_unlink(SHAREDMEMORY);
    return 0;
}










