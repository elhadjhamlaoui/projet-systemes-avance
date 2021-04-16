#ifndef __LPC_CLIENT__

#define __LPC_CLIENT__

typedef enum
{
    STRING,
    DOUBLE,
    INT,
    NOP
} lpc_type;

typedef struct
{
    int slen;
    char string[20];
} lpc_string;

void *lpc_open(const char *name);
int lpc_close(void *mem);
int lpc_call(void *memory, const char *fun_name, ...);
lpc_string *lpc_make_string(const char *s, int taille);

#endif
