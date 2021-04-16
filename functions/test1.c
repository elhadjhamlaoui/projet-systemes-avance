#include <string.h>
#include <stdio.h>
#include "../memory.h"

int test1(LpcArg *args)
{

    FILE *fp;

    fp = fopen(" GeeksForGeeks.txt ", "r");

    if (fp == NULL)
    {
        return -1;
    }

    // int i = 0;
    // while (args[i].type != NOP)
    // {
    //     switch (args[i].type)
    //     {
    //     case INT:
    //         args[i].intg = args[i].intg * 2;
    //         break;
    //     case DOUBLE:
    //         args[i].dbl = args[i].dbl * 2;
    //         break;
    //     case STRING:
    //         strcat(args[i].str.string, " **modified");
    //         break;
    //     default:
    //         break;
    //     }
    //     i++;
    // }
    //exit(0);
}