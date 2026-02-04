#ifndef CONFIG_H
#define CONFIG_H

#include <drivers/storage.h>
#include <utils.h>

#define MAX_VAR_NAME 32
#define MAX_VAR_VALUE 32
#define MAX_CONFIG_VARS 32

#define CONFIG_PATH ("System/system.ini")

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
