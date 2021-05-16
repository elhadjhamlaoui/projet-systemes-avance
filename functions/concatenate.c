

#include <string.h>
#include <stdio.h>
#include "../memory.h"
#include "../client.h"
#include <unistd.h>

int concatenate(LpcArg *args)
{
            if (((int)strlen(args[0].str.string) + (int)strlen(args[1].str.string)) >= STRING_LENGHT)
            {
                errno = ENOMEM;
                args[0].str.slen = -1;
                args[1].str.slen = -1;
                return -1;
            }
            strcat(args[0].str.string, args[1].str.string);

    return 0;
}