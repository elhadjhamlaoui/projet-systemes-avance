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

int main(int argc, char **argv)
{
    int a = 0;
    double b = 0;

    void *memory = lpc_open("/test2");
    lpc_string *str;
    char fun_name[NAMELEN];

    void *params[argc];
    for (int i = 1; i < argc; i++)
    {

        params[i] = &a;

        if (strcmp(argv[i], "int") == 0)
        {
            lpc_type type = INT;
            params[i] = &type;
            i++;
            a = strtol(argv[i], NULL, 0);
            params[i] = &a;
        }
        else if (strcmp(argv[i], "double") == 0)
        {
            lpc_type type = DOUBLE;
            params[i] = &type;
            i++;
            b = strtod(argv[i], NULL);
            params[i] = &b;
        }
        else if (strcmp(argv[i], "string") == 0)
        {
            lpc_type type = STRING;
            params[i] = &type;
            i++;
            str = lpc_make_string(argv[i], strlen(argv[i]) + 1);
            params[i] = str;
        }
        else if (strcmp(argv[i], "nop") == 0)
        {
            lpc_type type = NOP;
            params[i] = &type;
        }

        else
        {
            params[i] = argv[i];
            
            strcpy(fun_name, params[i]);
        }
    }

    lpc_call(memory, fun_name, INT, &a, DOUBLE, &b, STRING, str, NOP);

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

    va_list va;

    va_start(va, 1);
    int lastParam = 0;

    int i = 0;
    int j = 0;

    strcpy(mem->data.fun_name, fun_name);

    while (lastParam != 1)
    {
        lpc_type nextType = va_arg(va, lpc_type);
        j++;
        switch (nextType)
        {
        case STRING:
        {
            lpc_string *str = va_arg(va, lpc_string *);
            mem->data.lpcArgs[i].type = STRING;
            mem->data.lpcArgs[i].str = *str;
            j++;
        }

        break;
        case DOUBLE:
        {
            double *dbl = va_arg(va, double *);
            mem->data.lpcArgs[i].type = DOUBLE;

            mem->data.lpcArgs[i].dbl = *dbl;

            j++;
        }

        break;
        case INT:
        {

            int *intg = va_arg(va, int *);
            mem->data.lpcArgs[i].type = INT;

            mem->data.lpcArgs[i].intg = *intg;

            j++;
        }
        break;
        case NOP:
        {
            mem->data.lpcArgs[i].type = NOP;

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

    MEMORY *mm = (MEMORY *)memory;

    printf("\n***** BEFORE *****\n\n");
    printf("%d\n", mm->data.lpcArgs[0].intg);
    printf("%f\n", mm->data.lpcArgs[1].dbl);
    printf("%s\n", mm->data.lpcArgs[2].str.string);

    /* signaler le server */
    pthread_cond_signal(&mem->header.rcond);

    pthread_cond_wait(&mem->header.wcond, &mem->header.mutex);

    if (mem->header.result == -1)
    {
        errno = mem->header.err;
        printf("The error message is : %s\n", strerror(errno));
    }

    va_start(va, 1);

    printf("\n***** AFTER *****\n\n");

    lastParam = 0;
    for (int index = 0; index < j && lastParam == 0; index++)
    {
        va_arg(va, lpc_type);

        switch (mem->data.lpcArgs[index].type)
        {
        case STRING:
        {
            lpc_string str = mem->data.lpcArgs[index].str;
            lpc_string *param = va_arg(va, lpc_string *);
            strcpy(param->string, str.string);
            printf("%s\n", param->string);
        }

        break;
        case DOUBLE:
        {
            double dbl = mem->data.lpcArgs[index].dbl;
            double *param = va_arg(va, double *);
            memcpy(param, &dbl, sizeof(double));
            printf("%f\n", *param);
        }

        break;
        case INT:
        {
            int intg = mem->data.lpcArgs[index].intg;
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