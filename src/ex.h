#ifndef EX_H
#define EX_H

#include <alloc.h>
#include <scheduler.h>

#pragma pack(1)

typedef unsigned int DWORD;

typedef struct
{
        char name[16];
        int offset;
} Function;

typedef struct
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
} Header;

typedef struct
{
        int offset; /* final calculation is just value @off + new base */
} Relocation;

int execute(char *name, char *buffer, size_t buffer_size);

#endif
