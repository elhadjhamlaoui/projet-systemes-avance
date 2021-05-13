#include <string.h>
#include <stdio.h>
#include "../memory.h"
#include "../client.h"
#include <unistd.h>

int test2(LpcArg *args)
{

    // FILE *fp;

    // fp = fopen(" GeeksForGeeks.txt ", "r");

    // if (fp == NULL)
    // {
    //     return -1;
    // }

    int i = 0;
    while (args[i].type != NOP)
    {
        switch (args[i].type)
        {
        case INT:
            args[i].intg = args[i].intg * 2;
            break;
        case DOUBLE:
            args[i].dbl = args[i].dbl * 2;
            break;
        case STRING:
        {
            char str[] = " **modified";
            if (((int)strlen(str) + (int)strlen(args[i].str.string)) >= STRING_LENGHT)
            {
                errno = ENOMEM;
                args[i].str.slen = -1;
                return -1;
            }
            strcat(args[i].str.string, " **modified");
            break;
        }
        default:
            break;
        }
        i++;
    }
    return 0;
}