#ifndef CONFIG_H
#define CONFIG_H

#include <drivers/storage.h>
#include <utils.h>

#define MAX_VAR_NAME 64
#define MAX_VAR_VALUE 256
#define MAX_CONFIG_VARS 32

#define CONFIG_PATH ("System/system.ini")

typedef struct
{
        char name[MAX_VAR_NAME];
        char value[MAX_VAR_VALUE];
} CONFIG_VAR;

typedef struct
{
        CONFIG_VAR vars[MAX_CONFIG_VARS];
        int count;
} CONFIGURATION;

CONFIGURATION ConfigRead(void);
const char *ConfigGet(CONFIGURATION *config, const char *name);

#endif
