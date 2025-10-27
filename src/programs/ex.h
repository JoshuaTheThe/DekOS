#ifndef EX_H
#define EX_H

#include <programs/scheduler.h>
#include <memory/alloc.h>
#include <memory/string.h>

typedef unsigned int DWORD;

typedef struct __attribute__((packed))
{
        char name[16];
        int offset;
} exFunction_t;

typedef struct __attribute__((packed))
{
        char Sign[8];
        char major, minor, patch;
        int RelocationCount;
        int RelocationsStart;
        int TextSegmentOrg;
        int TextSegmentSize;
        int FunTableOrg;
        int FunctionCount;
        int StackSize;
} exHeader_t;

typedef struct __attribute__((packed))
{
        int offset; /* final calculation is just value @off + new base */
} exRelocation_t;

int exExecute(char *name, char *buffer, size_t buffer_size);

#endif
