#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <malloc.h>
#include <pthread.h>
#include<assert.h>

typedef struct
{
    pthread_mutex_t mutex;
    char buff[];
}sharedmemory;

#define LINUX_2204
//#define LINUX_1604

#ifdef LINUX_1604
#define SHARED_PHOTO_ORIGIN "/home/riki/Linkstart_demo/LinkStart/read_photo_demo/qq.jpeg"
#endif

#ifdef LINUX_2204
#define SHARED_PHOTO_ORIGIN "/home/linkstart/CLionProjects/read_photo_demo/qq.jpeg"
#endif

int main()
{
    int read_dev_shm = shm_open("qq_photo", O_RDONLY,0666);
    struct stat buff;
    if (fstat(read_dev_shm, &buff) == -1)
        printf("fstat error\n");

    printf("size=%ld\n", buff.st_size);
    ftruncate(read_dev_shm, buff.st_size);
    sharedmemory * Shm_Pthoto = (sharedmemory *)malloc(buff.st_size);
    if (Shm_Pthoto == NULL)
    {
        printf("malloc fial\n");
        exit(0);
    }

    sharedmemory * read_mmap = (sharedmemory * )mmap(NULL, buff.st_size,
                    PROT_READ, MAP_SHARED, read_dev_shm, 0);
    //(*Shm_Pthoto)=(*read_mmap); //This is wrong
    memcpy(Shm_Pthoto, read_mmap, buff.st_size);

    int save = open(SHARED_PHOTO_ORIGIN, O_CREAT | O_RDWR, 0666);
    write(save,Shm_Pthoto->buff,buff.st_size - sizeof(Shm_Pthoto->mutex));
    close(save);
    //test

    printf("getchar pause\n");
    getchar();
    free(Shm_Pthoto);
    Shm_Pthoto = NULL;
    close(read_dev_shm);
    munmap(read_mmap, buff.st_size);
    shm_unlink(SHARED_PHOTO_ORIGIN);

    return 0;
}
