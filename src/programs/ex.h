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
        size_t RelocationCount;
        size_t RelocationsStart;
        size_t TextSegmentOrg;
        size_t TextSegmentSize;
        size_t FunTableOrg;
        size_t FunctionCount;
        size_t StackSize;
} exHeader_t;

typedef struct __attribute__((packed))
{
        int offset; /* final calculation is just value @off + new base */
} exRelocation_t;

int exExecute(char *name, char *buffer, size_t buffer_size);
void exApplyRelocations(const exHeader_t h, const exRelocation_t *relocations, char *raw);
void exFindRawData(const exHeader_t h, const char *buffer, char *raw);
void exFindRelocations(const exHeader_t h, const char *buffer, exRelocation_t *relocations);
void exFindFunctions(const exHeader_t h, const char *buffer, exFunction_t *func);
void exHeaderInfo(const exHeader_t h);

#endif
