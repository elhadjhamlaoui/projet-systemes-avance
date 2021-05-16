

#include <string.h>
#include <stdio.h>
#include "../memory.h"
#include "../client.h"
#include <unistd.h>

int readFile(LpcArg *args)
{

    int i = 0;
    while (args[i].type != NOP)
    {
        switch (args[i].type)
        {

        case STRING:
        {
            FILE *fp;

            fp = fopen(args[i].str.string, "r");

            if (fp == NULL)
            {
                return -1;
            }

            char str[] = " **read";
            if (((int)strlen(str) + (int)strlen(args[i].str.string)) >= STRING_LENGHT)
            {
                errno = ENOMEM;
                args[i].str.slen = -1;
                return -1;
            }
            strcat(args[i].str.string, " **read");

            break;
        }
        default:
            break;
        }
        i++;
    }

    return 0;
}