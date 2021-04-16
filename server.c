#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <errno.h>

#include "memory.h"
#include "server.h"
#include "functions/test1.c"
#include "functions/test2.c"

int main(int argc, char **argv)
{

    lpc_function functions[2];

    strcpy(functions[0].fun_name, "test1");
    functions[0].fun = (int (*)(void *))test1;

    strcpy(functions[1].fun_name, "test2");
    functions[1].fun = (int (*)(void *))test2;

    size_t n = 8;
    MEMORY *mem = lpc_create("/test2", n);

    while (1)
    {
        pthread_mutex_lock(&mem->header.mutex);

        pthread_cond_wait(&mem->header.rcond, &mem->header.mutex);

        for (int i = 0; i < 2; i++)
        {
            if (strcmp(functions[i].fun_name, mem->data.fun_name) == 0)
            {
                mem->header.result = functions[i].fun(&mem->data.lpcArgs);
                if (mem->header.result == -1)
                {
                    mem->header.err = errno;
                }
                break;
            }
        }
        /* signaler le client */

        pthread_mutex_unlock(&mem->header.mutex);

        pthread_cond_signal(&mem->header.wcond);
    }

    exit(0);
}

void *lpc_create(const char *name, size_t capacity)
{
    shm_unlink(name);

    int fd = shm_open(name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        printf("fd");

        return NULL;
    }

    ftruncate(fd, capacity * sizeof(MEMORY));

    MEMORY *memory = mmap(NULL, capacity * sizeof(MEMORY),
                          PROT_READ | PROT_WRITE,
                          MAP_SHARED, fd, 0);
    if (memory == MAP_FAILED)
    {
        printf("mem");

        return NULL;
    }

    initialiser_mutex(&memory->header.mutex);
    //  thread_error(__FILE__, __LINE__, code, "init_mutex");
    initialiser_cond(&memory->header.rcond);

    //    thread_error(__FILE__, __LINE__, code, "init_rcond");
    initialiser_cond(&memory->header.wcond);
    //     thread_error(__FILE__, __LINE__, code, "init_wcond");

    return memory;
}

int initialiser_mutex(pthread_mutex_t *pmutex)
{
    pthread_mutexattr_t mutexattr;
    int code;
    if ((code = pthread_mutexattr_init(&mutexattr)) != 0)
        return code;

    if ((code = pthread_mutexattr_setpshared(&mutexattr,
                                             PTHREAD_PROCESS_SHARED)) != 0)
        return code;
    code = pthread_mutex_init(pmutex, &mutexattr);
    return code;
}

int initialiser_cond(pthread_cond_t *pcond)
{
    pthread_condattr_t condattr;
    int code;
    if ((code = pthread_condattr_init(&condattr)) != 0)
        return code;
    if ((code = pthread_condattr_setpshared(&condattr, PTHREAD_PROCESS_SHARED)) != 0)
        return code;
    code = pthread_cond_init(pcond, &condattr);
    return code;
}