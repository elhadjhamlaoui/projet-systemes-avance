#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/file.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <errno.h>

#include "memory.h"
#include "client.h"
#include <unistd.h>

int main(int argc, char **argv)
{
    int a = 0;
    double b = 0;

    void *memory = lpc_open("/lpc");
    lpc_string *str;
    char fun_name[NAMELEN];

    void *params[argc];

    strcpy(fun_name, argv[1]);
    int j = 0;
    for (int i = 2; i < argc; i++)
    {
        if (strcmp(argv[i], "int") == 0)
        {
            i++;
            a = strtol(argv[i], NULL, 0);
            params[j] = &a;
        }
        else if (strcmp(argv[i], "double") == 0)
        {
            i++;
            b = strtod(argv[i], NULL);
            params[j] = &b;
        }
        else if (strcmp(argv[i], "string") == 0)
        {
            i++;
            str = lpc_make_string(argv[i], strlen(argv[i]) + 1);
            params[j] = str;
        }

        j++;
    }

    if (strcmp(argv[1], "multiply") == 0)
    {
        lpc_call(memory, fun_name, INT, params[0], DOUBLE, params[1], STRING, params[2], NOP);
    }
    else if (strcmp(argv[1], "multiply_sleep") == 0)
    {
        lpc_call(memory, fun_name, INT, params[0], DOUBLE, params[1], STRING, params[2], NOP);
    }
    else if (strcmp(argv[1], "readFile") == 0)
    {
        lpc_call(memory, fun_name, STRING, params[0], NOP);
    }
    else if (strcmp(argv[1], "concatenate") == 0)
    {
        lpc_call(memory, fun_name, STRING, params[0], STRING, params[1], NOP);
    }
}

lpc_string *lpc_make_string(const char *s, int taille)
{
    lpc_string *str;
    if (taille > 0 && s == NULL)
    {
        str = malloc(sizeof(lpc_string) + taille);
        str->slen = taille;
        memset(str->string, '\0', sizeof(char) * str->slen);
    }
    else if (taille <= 0 && s != NULL)
    {
        str = malloc(sizeof(lpc_string) + strlen(s) + 1);
        str->slen = strlen(s) + 1;
        strcpy(str->string, s);
    }
    else if (taille >= strlen(s) + 1)
    {
        str = malloc(sizeof(lpc_string));
        str->slen = taille;
        strcpy(str->string, s);
    }
    else
    {
        return NULL;
    }
    return str;
}

int lpc_call(void *memory, const char *fun_name, ...)
{
    MEMORY *mem = memory;

    /* section critique */

    mem->header.pid = getpid();

    /* signaler le server */

    pthread_mutex_unlock(&mem->header.mutex);

    pthread_cond_signal(&mem->header.rcond);

    pthread_mutex_lock(&mem->header.mutex);
    pthread_cond_wait(&mem->header.wcond, &mem->header.mutex);

    char str[50];
    sprintf(str, "/lpc%d", getpid());
    MEMORY *mem_communication = lpc_open(str);

    va_list va;

    va_start(va, fun_name);
    int lastParam = 0;

    int i = 0;
    int j = 0;

    strcpy(mem_communication->data.fun_name, fun_name);

    printf("\n***** BEFORE *****\n\n");

    while (lastParam != 1)
    {
        lpc_type nextType = va_arg(va, lpc_type);
        j++;
        switch (nextType)
        {
        case STRING:
        {
            lpc_string *str = va_arg(va, lpc_string *);
            mem_communication->data.lpcArgs[i].type = STRING;
            mem_communication->data.lpcArgs[i].str = *str;
            printf("%s\n", str->string);

            j++;
        }

        break;
        case DOUBLE:
        {
            double *dbl = va_arg(va, double *);
            mem_communication->data.lpcArgs[i].type = DOUBLE;

            mem_communication->data.lpcArgs[i].dbl = *dbl;
            printf("%f\n", *dbl);

            j++;
        }

        break;
        case INT:
        {
            int *intg = va_arg(va, int *);

            mem_communication->data.lpcArgs[i].type = INT;

            mem_communication->data.lpcArgs[i].intg = *intg;

            printf("%d\n", *intg);

            j++;
        }
        break;
        case NOP:
        {
            mem_communication->data.lpcArgs[i].type = NOP;

            j++;
            lastParam = 1;
        }
        break;

        default:
            lastParam = 1;
            break;
        }
        i++;
    }

    va_end(va);

    pthread_mutex_unlock(&mem->header.mutex);

    pthread_cond_signal(&mem_communication->header.rcond);

    pthread_mutex_lock(&mem->header.mutex);

    pthread_cond_wait(&mem_communication->header.wcond, &mem_communication->header.mutex);

    if (mem_communication->header.result == -1)
    {
        errno = mem_communication->header.err;
        printf("The error message is : %s\n", strerror(errno));
        return -1;
    }

    printf("\n***** AFTER *****\n\n");

    va_start(va, fun_name);

    lastParam = 0;
    for (int index = 0; index < j && lastParam == 0; index++)
    {
        va_arg(va, lpc_type);

        switch (mem_communication->data.lpcArgs[index].type)
        {
        case STRING:
        {
            lpc_string str = mem_communication->data.lpcArgs[index].str;
            lpc_string *param = va_arg(va, lpc_string *);
            strcpy(param->string, str.string);
            printf("%s\n", param->string);
        }

        break;
        case DOUBLE:
        {
            double dbl = mem_communication->data.lpcArgs[index].dbl;
            double *param = va_arg(va, double *);
            memcpy(param, &dbl, sizeof(double));
            printf("%f\n", *param);
        }

        break;
        case INT:
        {
            int intg = mem_communication->data.lpcArgs[index].intg;
            int *param = va_arg(va, int *);
            memcpy(param, &intg, sizeof(int));
            printf("%d\n", *param);
        }

        break;

        case NOP:
        {
            lastParam = 1;
        }

        break;
        default:
            lastParam = 1;

            break;
        }
    }
    va_end(va);

    return 0;
}

void *lpc_open(const char *name)
{

    int fd = shm_open(name, O_RDWR, S_IRUSR | S_IWUSR);
    // if (fd < 0)
    //     PANIC_EXIT("shm_open");

    //projection en mémoire
    MEMORY *mem;
    mem = mmap(NULL, sizeof(MEMORY), PROT_READ | PROT_WRITE,
               MAP_SHARED, fd, 0);

    return mem;
}

int lpc_close(void *mem)
{
    return munmap(mem, sizeof(MEMORY));
}
