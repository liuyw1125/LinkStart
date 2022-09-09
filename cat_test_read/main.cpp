#include <sys/mman.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>

typedef struct{
    pthread_mutex_t pmutex;
    unsigned char data[20];
}sharedmemory;
#define SHAREDMEMORYFILE "shm_file"

int main()
{
    int fd = shm_open(SHAREDMEMORYFILE, O_RDWR, 0777);
    sharedmemory *ptr = (sharedmemory *)mmap(NULL, 1024, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (ptr == MAP_FAILED)
    {
        printf("open shared failed \n");
        exit(0);
    }
    pthread_mutex_lock(&ptr->pmutex);
    for (unsigned char i = 0; i < 20; i++)
    {
        printf("%d ", ptr->data[i]);
    }
    pthread_mutex_unlock(&ptr->pmutex);
    printf("get end\n");
    getchar();
    munmap(ptr, 1024);
    return 0;
}
