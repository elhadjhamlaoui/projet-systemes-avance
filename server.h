#ifndef __LPC_SERVER__

#define __LPC_SERVER__

#define NAMELEN 48
typedef struct
{
    char fun_name[NAMELEN];
    int (*fun)(void *);
} lpc_function;


void *lpc_create(const char *name, size_t n);
int initialiser_mutex(pthread_mutex_t *pmutex);
int initialiser_cond(pthread_cond_t *pcond);

#endif

