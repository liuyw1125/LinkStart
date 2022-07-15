#ifndef POSIXSHM_H
#define POSIXSHM_H

#include <sys/types.h>
#include <sys/stat.h>        /* For mode constants */
#include <sys/mman.h>
#include <fcntl.h>           /* For O_* constants */
#include <unistd.h>

using namespace std;

class PosixShm
{
public:
    //
    PosixShm();

};

#endif
