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
#define SHARED_PHOTO_ORIGIN "/home/riki/Linkstart_demo/LinkStart/copy_photo_demo/qq.jpeg"
#define SHARED_PHOTO_TEST "/home/riki/Linkstart_demo/LinkStart/copy_photo_demo/qq_test.jpeg"
#define SHARED_PHOTO_SHM_TEST "/home/riki/Linkstart_demo/LinkStart/copy_photo_demo/qq_shm_test.jpeg"
#endif

#ifdef LINUX_2204
#define SHARED_PHOTO_ORIGIN "/home/linkstart/CLionProjects/copy_photo_demo/qq.jpeg"
#define SHARED_PHOTO_TEST "/home/linkstart/CLionProjects/copy_photo_demo/qq_test.jpeg"
#define SHARED_PHOTO_SHM_TEST "/home/linkstart/CLionProjects/copy_photo_demo/qq_shm_test.jpeg"
#endif


int main()
{
    int fd_read = open(SHARED_PHOTO_ORIGIN,O_RDONLY);
    if (fd_read == -1)
    {
        printf("open error!\n");
        exit(0);
    }
    int jpeg_size = lseek(fd_read,0,SEEK_END);
    printf("jpeg size = %d\n", jpeg_size);
    printf("fd_read size = %d \n", jpeg_size +  (int)sizeof(sharedmemory));
    lseek(fd_read, 0 , SEEK_SET);
    sharedmemory * Shm_Pthoto = (sharedmemory *)malloc(sizeof(sharedmemory) + jpeg_size);
    if (Shm_Pthoto == NULL)
    {
        printf("malloc fial\n");
        exit(0);
    }
    int read_size = read(fd_read, Shm_Pthoto->buff, jpeg_size);
    printf("read size = %d\n", read_size);

    int write_test = open(SHARED_PHOTO_TEST, O_CREAT | O_RDWR,0666);
    write(write_test,Shm_Pthoto->buff,jpeg_size);
    close(write_test);

    int fd_dev_shm = shm_open("qq_photo", O_CREAT | O_RDWR,0666);
    ftruncate(fd_dev_shm, sizeof(sharedmemory) + jpeg_size);
    sharedmemory * dev_Shm = (sharedmemory *)mmap(NULL, sizeof(sharedmemory)+jpeg_size,PROT_READ | PROT_WRITE, MAP_SHARED, fd_dev_shm, 0);
   // (*dev_Shm) = (*Shm_Pthoto);
   //memcpy(dev_Shm->buff, Shm_Pthoto->buff, jpeg_size);
    memcpy(dev_Shm, Shm_Pthoto, sizeof(sharedmemory) + jpeg_size);

    int write_shm_test = open(SHARED_PHOTO_SHM_TEST, O_CREAT | O_RDWR,0666);
    write(write_shm_test,dev_Shm->buff,jpeg_size);
    close(write_shm_test);

    printf("getchar pause\n");
    getchar();
    free(Shm_Pthoto);
    Shm_Pthoto = NULL;
    close(fd_read);
    close(fd_dev_shm);
    munmap(dev_Shm, sizeof(sharedmemory)+jpeg_size);
    shm_unlink("qq_photo");

    return 0;
}
