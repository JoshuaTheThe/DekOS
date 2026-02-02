#ifndef INI_H
#define INI_H

#include <stdio.h>
#include <string.h>

#define MAX_VAR_NAME 64
#define MAX_VAR_VALUE 256
#define MAX_CONFIG_VARS 32

typedef struct
{
        char name[MAX_VAR_NAME];
        char value[MAX_VAR_VALUE];
} IniVar;

typedef struct
{
        IniVar vars[MAX_CONFIG_VARS];
        int count;
} Ini;

Ini IniRead(const char *Path);
const char *IniGet(Ini *config, const char *name);

#endif
