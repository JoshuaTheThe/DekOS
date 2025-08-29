#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>

extern int memcmp(const void*,const void*,int);
extern int memcpy(void*,const void*,int);
extern int strncmp(const void*,const void*,int);
extern void *malloc(int);
extern void free(void*);
extern void memory_init(void);
extern char *strtok(char *str, const char *delim);
extern char *strchr(const char *str, int c);

size_t strlen(const char *str);
char *strncpy(char *dest, const char *src, size_t n);
char *strrchr(const char *s, int c);
int strcmp(const char *s1, const char *s2);
char *strtok(char *str, const char *delim);
char *strchr(const char *s, int c);
int toupper(int c);
int tolower(int c);

// Memory allocation structures
typedef struct mem_block
{
        struct mem_block *next;
        size_t size;
        int free;
} mem_block_t;

// Heap boundaries (set by linker script or startup code)
extern uint8_t _heap_start[];
extern uint8_t _heap_end[];

#endif
