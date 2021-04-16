#ifndef __LPC_MEMORY__

#define __LPC_MEMORY__

#include <semaphore.h>
#include "client.h"
#include <pthread.h>



#define NAMELEN 48
#define LPC_ARG_MAX 20

/* Define a structure that will be imposed on the shared
              memory object */


typedef struct
{
    lpc_type type;
    int intg;
    double dbl;
    lpc_string str;
} LpcArg;

typedef struct
{
    pthread_mutex_t mutex;
    pthread_cond_t rcond;
    pthread_cond_t wcond;
    int result;
    int err;
} HEADER;

typedef struct
{
    char fun_name[NAMELEN];
    LpcArg lpcArgs[LPC_ARG_MAX];
} DATA;

typedef struct
{
    HEADER header;
    DATA data;
} MEMORY;

#endif
